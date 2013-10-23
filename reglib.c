#include <errno.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

#include <arpa/inet.h> /* ntohl */

#include "reglib.h"
#include "regdb.h"

#ifdef USE_OPENSSL
#include <openssl/objects.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>
#include <openssl/pem.h>
#endif

#ifdef USE_GCRYPT
#include <gcrypt.h>
#endif

#include "reglib.h"

#ifdef USE_OPENSSL
#include "keys-ssl.c"
#endif

#ifdef USE_GCRYPT
#include "keys-gcrypt.c"
#endif

void *
reglib_get_file_ptr(uint8_t *db, size_t dblen, size_t structlen, uint32_t ptr)
{
	uint32_t p = ntohl(ptr);

	if (structlen > dblen) {
		fprintf(stderr, "Invalid database file, too short!\n");
		exit(3);
	}

	if (p > dblen - structlen) {
		fprintf(stderr, "Invalid database file, bad pointer!\n");
		exit(3);
	}

	return (void *)(db + p);
}

static size_t
reglib_array_len(size_t baselen, unsigned int elemcount, size_t elemlen)
{
	if (elemcount > (SIZE_MAX - baselen) / elemlen) {
		fprintf(stderr, "Invalid database file, count too large!\n");
		exit(3);
	}

	return baselen + elemcount * elemlen;
}

/*
 * reglib_verify_db_signature():
 *
 * Checks the validity of the signature found on the regulatory
 * database against the array 'keys'. Returns 1 if there exists
 * at least one key in the array such that the signature is valid
 * against that key; 0 otherwise.
 */

#ifdef USE_OPENSSL
int reglib_verify_db_signature(uint8_t *db, size_t dblen, size_t siglen)
{
	RSA *rsa;
	uint8_t hash[SHA_DIGEST_LENGTH];
	unsigned int i;
	int ok = 0;
	DIR *pubkey_dir;
	struct dirent *nextfile;
	FILE *keyfile;
	char filename[PATH_MAX];

	if (SHA1(db, dblen, hash) != hash) {
		fprintf(stderr, "Failed to calculate SHA1 sum.\n");
		goto out;
	}

	for (i = 0; (i < sizeof(keys)/sizeof(keys[0])) && (!ok); i++) {
		rsa = RSA_new();
		if (!rsa) {
			fprintf(stderr, "Failed to create RSA key.\n");
			goto out;
		}

		rsa->e = &keys[i].e;
		rsa->n = &keys[i].n;

		ok = RSA_verify(NID_sha1, hash, SHA_DIGEST_LENGTH,
				db + dblen, siglen, rsa) == 1;

		rsa->e = NULL;
		rsa->n = NULL;
		RSA_free(rsa);
	}
	if (!ok && (pubkey_dir = opendir(PUBKEY_DIR))) {
		while (!ok && (nextfile = readdir(pubkey_dir))) {
			snprintf(filename, PATH_MAX, "%s/%s", PUBKEY_DIR,
				nextfile->d_name);
			if ((keyfile = fopen(filename, "rb"))) {
				rsa = PEM_read_RSA_PUBKEY(keyfile,
					NULL, NULL, NULL);
				if (rsa)
					ok = RSA_verify(NID_sha1, hash, SHA_DIGEST_LENGTH,
						db + dblen, siglen, rsa) == 1;
				RSA_free(rsa);
				fclose(keyfile);
			}
		}
		closedir(pubkey_dir);
	}

	if (!ok)
		fprintf(stderr, "Database signature verification failed.\n");

out:
	return ok;
}
#endif /* USE_OPENSSL */

#ifdef USE_GCRYPT
int reglib_verify_db_signature(uint8_t *db, size_t dblen, size_t siglen)
{
	gcry_mpi_t mpi_e, mpi_n;
	gcry_sexp_t rsa, signature, data;
	uint8_t hash[20];
	unsigned int i;
	int ok = 0;

	/* initialise */
	gcry_check_version(NULL);

	/* hash the db */
	gcry_md_hash_buffer(GCRY_MD_SHA1, hash, db, dblen);

	if (gcry_sexp_build(&data, NULL, "(data (flags pkcs1) (hash sha1 %b))",
			    20, hash)) {
		fprintf(stderr, "Failed to build data S-expression.\n");
		return ok;
	}

	if (gcry_sexp_build(&signature, NULL, "(sig-val (rsa (s %b)))",
			    siglen, db + dblen)) {
		fprintf(stderr, "Failed to build signature S-expression.\n");
		gcry_sexp_release(data);
		return ok;
	}

	for (i = 0; (i < sizeof(keys)/sizeof(keys[0])) && (!ok); i++) {
		if (gcry_mpi_scan(&mpi_e, GCRYMPI_FMT_USG,
				keys[i].e, keys[i].len_e, NULL) ||
		    gcry_mpi_scan(&mpi_n, GCRYMPI_FMT_USG,
				keys[i].n, keys[i].len_n, NULL)) {
			fprintf(stderr, "Failed to convert numbers.\n");
			goto out;
		}

		if (gcry_sexp_build(&rsa, NULL,
				    "(public-key (rsa (n %m) (e %m)))",
				    mpi_n, mpi_e)) {
			fprintf(stderr, "Failed to build RSA S-expression.\n");
			gcry_mpi_release(mpi_e);
			gcry_mpi_release(mpi_n);
			goto out;
		}

		ok = gcry_pk_verify(signature, data, rsa) == 0;
		gcry_mpi_release(mpi_e);
		gcry_mpi_release(mpi_n);
		gcry_sexp_release(rsa);
	}

	if (!ok)
		fprintf(stderr, "Database signature verification failed.\n");

out:
	gcry_sexp_release(data);
	gcry_sexp_release(signature);
	return ok;
}
#endif /* USE_GCRYPT */

#if !defined(USE_OPENSSL) && !defined(USE_GCRYPT)
int reglib_verify_db_signature(uint8_t *db, size_t dblen, size_t siglen)
{
	return 1;
}
#endif

const struct reglib_regdb_ctx *reglib_malloc_regdb_ctx(const char *regdb_file)
{
	struct regdb_file_header *header;
	struct reglib_regdb_ctx *ctx;

	ctx = malloc(sizeof(struct reglib_regdb_ctx));
	if (!ctx)
		return NULL;

	memset(ctx, 0, sizeof(struct reglib_regdb_ctx));

	ctx->fd = open(regdb_file, O_RDONLY);

	if (ctx->fd < 0) {
		free(ctx);
		return NULL;
	}

	if (fstat(ctx->fd, &ctx->stat)) {
		close(ctx->fd);
		free(ctx);
		return NULL;
	}

	ctx->real_dblen = ctx->stat.st_size;

	ctx->db = mmap(NULL, ctx->real_dblen, PROT_READ,
		       MAP_PRIVATE, ctx->fd, 0);
	if (ctx->db == MAP_FAILED) {
		close(ctx->fd);
		free(ctx);
		return NULL;
	}

	ctx->header = reglib_get_file_ptr(ctx->db, ctx->real_dblen,
					  sizeof(struct regdb_file_header),
					  0);
	header = ctx->header;

	if (ntohl(header->magic) != REGDB_MAGIC)
		goto err_out;

	if (ntohl(header->version) != REGDB_VERSION)
		goto err_out;

	ctx->siglen = ntohl(header->signature_length);

	if (ctx->siglen > ctx->real_dblen - sizeof(*header))
		goto err_out;

	/* The actual dblen does not take into account the signature */
	ctx->dblen = ctx->real_dblen - ctx->siglen;

	/* verify signature */
	if (!reglib_verify_db_signature(ctx->db, ctx->dblen, ctx->siglen))
		goto err_out;

	ctx->verified = true;
	ctx->num_countries = ntohl(header->reg_country_num);
	ctx->countries = reglib_get_file_ptr(ctx->db,
					     ctx->dblen,
					     sizeof(struct regdb_file_reg_country) * ctx->num_countries,
					     header->reg_country_ptr);
	return ctx;

err_out:
	close(ctx->fd);
	munmap(ctx->db, ctx->real_dblen);
	free(ctx);
	return NULL;
}

void reglib_free_regdb_ctx(const struct reglib_regdb_ctx *regdb_ctx)
{
	struct reglib_regdb_ctx *ctx;

	if (!regdb_ctx)
		return;

	ctx = (struct reglib_regdb_ctx *) regdb_ctx;

	memset(ctx, 0, sizeof(struct reglib_regdb_ctx));
	close(ctx->fd);
	munmap(ctx->db, ctx->real_dblen);
	free(ctx);
}

static void reg_rule2rd(uint8_t *db, size_t dblen,
	uint32_t ruleptr, struct ieee80211_reg_rule *rd_reg_rule)
{
	struct regdb_file_reg_rule *rule;
	struct regdb_file_freq_range *freq;
	struct regdb_file_power_rule *power;

	struct ieee80211_freq_range *rd_freq_range = &rd_reg_rule->freq_range;
	struct ieee80211_power_rule *rd_power_rule = &rd_reg_rule->power_rule;

	rule  = reglib_get_file_ptr(db, dblen, sizeof(*rule), ruleptr);
	freq  = reglib_get_file_ptr(db, dblen, sizeof(*freq), rule->freq_range_ptr);
	power = reglib_get_file_ptr(db, dblen, sizeof(*power), rule->power_rule_ptr);

	rd_freq_range->start_freq_khz = ntohl(freq->start_freq);
	rd_freq_range->end_freq_khz = ntohl(freq->end_freq);
	rd_freq_range->max_bandwidth_khz = ntohl(freq->max_bandwidth);

	rd_power_rule->max_antenna_gain = ntohl(power->max_antenna_gain);
	rd_power_rule->max_eirp = ntohl(power->max_eirp);

	rd_reg_rule->flags = ntohl(rule->flags);

	if (rd_reg_rule->flags & RRF_NO_IR_ALL)
		rd_reg_rule->flags |= RRF_NO_IR_ALL;
}

/* Converts a file regdomain to ieee80211_regdomain, easier to manage */
const static struct ieee80211_regdomain *
country2rd(const struct reglib_regdb_ctx *ctx,
	   struct regdb_file_reg_country *country)
{
	struct regdb_file_reg_rules_collection *rcoll;
	struct ieee80211_regdomain *rd;
	unsigned int i, num_rules;
	size_t size_of_rd;

	rcoll = reglib_get_file_ptr(ctx->db, ctx->dblen, sizeof(*rcoll),
				    country->reg_collection_ptr);
	num_rules = ntohl(rcoll->reg_rule_num);
	/* re-get pointer with sanity checking for num_rules */
	rcoll = reglib_get_file_ptr(ctx->db, ctx->dblen,
				    reglib_array_len(sizeof(*rcoll), num_rules,
						     sizeof(uint32_t)),
				    country->reg_collection_ptr);

	size_of_rd = reglib_array_len(sizeof(struct ieee80211_regdomain),
				      num_rules,
				      sizeof(struct ieee80211_reg_rule));

	rd = malloc(size_of_rd);
	if (!rd)
		return NULL;

	memset(rd, 0, size_of_rd);

	rd->alpha2[0] = country->alpha2[0];
	rd->alpha2[1] = country->alpha2[1];
	rd->dfs_region = country->creqs & 0x3;
	rd->n_reg_rules = num_rules;

	for (i = 0; i < num_rules; i++) {
		reg_rule2rd(ctx->db, ctx->dblen, rcoll->reg_rule_ptrs[i],
			&rd->reg_rules[i]);
	}

	return rd;
}

const struct ieee80211_regdomain *
reglib_get_rd_idx(unsigned int idx, const struct reglib_regdb_ctx *ctx)
{
	struct regdb_file_reg_country *country;

	if (!ctx)
		return NULL;

	if (idx >= ctx->num_countries)
		return NULL;

	country = ctx->countries + idx;

	return country2rd(ctx, country);
}

const struct ieee80211_regdomain *
reglib_get_rd_alpha2(const char *alpha2, const char *file)
{
	const struct reglib_regdb_ctx *ctx;
	const struct ieee80211_regdomain *rd = NULL;
	struct regdb_file_reg_country *country;
	bool found_country = false;
	unsigned int i;

	ctx = reglib_malloc_regdb_ctx(file);
	if (!ctx)
		return NULL;

	for (i = 0; i < ctx->num_countries; i++) {
		country = ctx->countries + i;
		if (memcmp(country->alpha2, alpha2, 2) == 0) {
			found_country = 1;
			break;
		}
	}

	if (!found_country)
		goto out;

	rd = country2rd(ctx, country);
	if (!rd)
		goto out;

out:
	reglib_free_regdb_ctx(ctx);
	return rd;
}

/* Sanity check on a regulatory rule */
static int is_valid_reg_rule(const struct ieee80211_reg_rule *rule)
{
	const struct ieee80211_freq_range *freq_range = &rule->freq_range;
	uint32_t freq_diff;

	if (freq_range->start_freq_khz == 0 || freq_range->end_freq_khz == 0)
		return 0;

	if (freq_range->start_freq_khz > freq_range->end_freq_khz)
		return 0;

	freq_diff = freq_range->end_freq_khz - freq_range->start_freq_khz;

	if (freq_range->end_freq_khz <= freq_range->start_freq_khz ||
	    freq_range->max_bandwidth_khz > freq_diff)
		return 0;

	return 1;
}

int reglib_is_valid_rd(const struct ieee80211_regdomain *rd)
{
	const struct ieee80211_reg_rule *reg_rule = NULL;
	unsigned int i;

	if (!rd->n_reg_rules)
		return 0;

	for (i = 0; i < rd->n_reg_rules; i++) {
		reg_rule = &rd->reg_rules[i];
		if (!is_valid_reg_rule(reg_rule))
		return 0;
	}
	return 1;
}

/*
 * Helper for reglib_intersect_rds(), this does the real
 * mathematical intersection fun
 */
static int reg_rules_intersect(const struct ieee80211_reg_rule *rule1,
			       const struct ieee80211_reg_rule *rule2,
			       struct ieee80211_reg_rule *intersected_rule)
{
	const struct ieee80211_freq_range *freq_range1, *freq_range2;
	struct ieee80211_freq_range *freq_range;
	const struct ieee80211_power_rule *power_rule1, *power_rule2;
	struct ieee80211_power_rule *power_rule;
	uint32_t freq_diff;

	freq_range1 = &rule1->freq_range;
	freq_range2 = &rule2->freq_range;
	freq_range = &intersected_rule->freq_range;

	power_rule1 = &rule1->power_rule;
	power_rule2 = &rule2->power_rule;
	power_rule = &intersected_rule->power_rule;

	freq_range->start_freq_khz = reglib_max(freq_range1->start_freq_khz,
					 freq_range2->start_freq_khz);
	freq_range->end_freq_khz = reglib_min(freq_range1->end_freq_khz,
				       freq_range2->end_freq_khz);
	freq_range->max_bandwidth_khz = reglib_min(freq_range1->max_bandwidth_khz,
					    freq_range2->max_bandwidth_khz);

	freq_diff = freq_range->end_freq_khz - freq_range->start_freq_khz;
	if (freq_range->max_bandwidth_khz > freq_diff)
		freq_range->max_bandwidth_khz = freq_diff;

	power_rule->max_eirp = reglib_min(power_rule1->max_eirp,
		power_rule2->max_eirp);
	power_rule->max_antenna_gain = reglib_min(power_rule1->max_antenna_gain,
		power_rule2->max_antenna_gain);

	intersected_rule->flags = rule1->flags | rule2->flags;

	if (!is_valid_reg_rule(intersected_rule))
		return -EINVAL;

	return 0;
}

/**
 * reglib_intersect_rds - do the intersection between two regulatory domains
 * @rd1: first regulatory domain
 * @rd2: second regulatory domain
 *
 * Use this function to get the intersection between two regulatory domains.
 * Once completed we will mark the alpha2 for the rd as intersected, "98",
 * as no one single alpha2 can represent this regulatory domain.
 *
 * Returns a pointer to the regulatory domain structure which will hold the
 * resulting intersection of rules between rd1 and rd2. We will
 * malloc() this structure for you.
 */
struct ieee80211_regdomain *
reglib_intersect_rds(const struct ieee80211_regdomain *rd1,
		     const struct ieee80211_regdomain *rd2)
{
	int r;
	size_t size_of_regd;
	unsigned int x, y;
	unsigned int num_rules = 0, rule_idx = 0;
	const struct ieee80211_reg_rule *rule1, *rule2;
	struct ieee80211_reg_rule *intersected_rule;
	struct ieee80211_regdomain *rd;
	/* This is just a dummy holder to help us count */
	struct ieee80211_reg_rule irule;

	/* Uses the stack temporarily for counter arithmetic */
	intersected_rule = &irule;

	memset(intersected_rule, 0, sizeof(struct ieee80211_reg_rule));

	if (!rd1 || !rd2)
		return NULL;

	/* First we get a count of the rules we'll need, then we actually
	 * build them. This is to so we can malloc() and free() a
	 * regdomain once. The reason we use reg_rules_intersect() here
	 * is it will return -EINVAL if the rule computed makes no sense.
	 * All rules that do check out OK are valid. */

	for (x = 0; x < rd1->n_reg_rules; x++) {
		rule1 = &rd1->reg_rules[x];
		for (y = 0; y < rd2->n_reg_rules; y++) {
			rule2 = &rd2->reg_rules[y];
			if (!reg_rules_intersect(rule1, rule2,
					intersected_rule))
				num_rules++;
			memset(intersected_rule, 0,
					sizeof(struct ieee80211_reg_rule));
		}
	}

	if (!num_rules)
		return NULL;

	size_of_regd = reglib_array_len(sizeof(struct ieee80211_regdomain),
					num_rules + 1,
					sizeof(struct ieee80211_reg_rule));

	rd = malloc(size_of_regd);
	if (!rd)
		return NULL;

	memset(rd, 0, size_of_regd);

	for (x = 0; x < rd1->n_reg_rules; x++) {
		rule1 = &rd1->reg_rules[x];
		for (y = 0; y < rd2->n_reg_rules; y++) {
			rule2 = &rd2->reg_rules[y];
			/* This time around instead of using the stack lets
			 * write to the target rule directly saving ourselves
			 * a memcpy() */
			intersected_rule = &rd->reg_rules[rule_idx];
			r = reg_rules_intersect(rule1, rule2,
				intersected_rule);
			if (r)
				continue;
			rule_idx++;
		}
	}

	if (rule_idx != num_rules) {
		free(rd);
		return NULL;
	}

	rd->n_reg_rules = num_rules;
	rd->alpha2[0] = '9';
	rd->alpha2[1] = '9';

	return rd;
}

const struct ieee80211_regdomain *
reglib_intersect_regdb(const struct reglib_regdb_ctx *ctx)
{
	const struct ieee80211_regdomain *rd;
	struct ieee80211_regdomain *prev_rd_intsct = NULL, *rd_intsct = NULL;
	int intersected = 0;
	unsigned int idx = 0;

	if (!ctx)
		return NULL;

	reglib_for_each_country(rd, idx, ctx) {
		if (reglib_is_world_regdom((const char *) rd->alpha2)) {
			free((struct ieee80211_regdomain *) rd);
			continue;
		}

		if (!prev_rd_intsct) {
			prev_rd_intsct = (struct ieee80211_regdomain *) rd;
			continue;
		}

		if (rd_intsct) {
			free(prev_rd_intsct);
			prev_rd_intsct = (struct ieee80211_regdomain *) rd_intsct;
		}

		rd_intsct = reglib_intersect_rds(prev_rd_intsct, rd);
		if (!rd_intsct) {
			free(prev_rd_intsct);
			free((struct ieee80211_regdomain *) rd);
			return NULL;
		}

		intersected++;
		free((struct ieee80211_regdomain *) rd);
	}

	if (!idx)
		return NULL;

	if (intersected <= 0) {
		rd_intsct = prev_rd_intsct;
		prev_rd_intsct = NULL;
		if (idx > 1) {
			free(rd_intsct);
			return NULL;
		}
	}

	if (prev_rd_intsct)
		free(prev_rd_intsct);

	return rd_intsct;
}

static const char *dfs_domain_name(enum regdb_dfs_regions region)
{
	switch (region) {
	case REGDB_DFS_UNSET:
		return "DFS-UNSET";
	case REGDB_DFS_FCC:
		return "DFS-FCC";
	case REGDB_DFS_ETSI:
		return "DFS-ETSI";
	case REGDB_DFS_JP:
		return "DFS-JP";
	default:
		return "DFS-invalid";
	}
}

static void print_reg_rule(const struct ieee80211_reg_rule *rule)
{
	const struct ieee80211_freq_range *freq;
	const struct ieee80211_power_rule *power;

	freq  = &rule->freq_range;
	power = &rule->power_rule;

	printf("\t(%.3f - %.3f @ %.3f), ",
	       ((float)(freq->start_freq_khz))/1000.0,
	       ((float)(freq->end_freq_khz))/1000.0,
	       ((float)(freq->max_bandwidth_khz))/1000.0);

	printf("(");

	if (power->max_eirp)
		printf("%.2f)", ((float)(power->max_eirp)/100.0));
	else
		printf("N/A)");

	if (rule->flags & RRF_NO_OFDM)
		printf(", NO-OFDM");
	if (rule->flags & RRF_NO_CCK)
		printf(", NO-CCK");
	if (rule->flags & RRF_NO_INDOOR)
		printf(", NO-INDOOR");
	if (rule->flags & RRF_NO_OUTDOOR)
		printf(", NO-OUTDOOR");
	if (rule->flags & RRF_DFS)
		printf(", DFS");
	if (rule->flags & RRF_PTP_ONLY)
		printf(", PTP-ONLY");
	if (rule->flags & RRF_PTMP_ONLY)
		printf(", PTMP-ONLY");
	if (rule->flags & RRF_NO_IR_ALL)
		printf(", NO-IR");

	printf("\n");
}

void reglib_print_regdom(const struct ieee80211_regdomain *rd)
{
	unsigned int i;
	printf("country %.2s: %s\n", rd->alpha2,
	       dfs_domain_name(rd->dfs_region));
	for (i = 0; i < rd->n_reg_rules; i++)
		print_reg_rule(&rd->reg_rules[i]);
	printf("\n");
}
