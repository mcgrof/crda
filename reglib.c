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

void *crda_get_file_ptr(uint8_t *db, int dblen, int structlen, uint32_t ptr)
{
	uint32_t p = ntohl(ptr);

	if (p > dblen - structlen) {
		fprintf(stderr, "Invalid database file, bad pointer!\n");
		exit(3);
	}

	return (void *)(db + p);
}

/*
 * Checks the validity of the signature found on the regulatory
 * database against the array 'keys'. Returns 1 if there exists
 * at least one key in the array such that the signature is valid
 * against that key; 0 otherwise.
 */
int crda_verify_db_signature(uint8_t *db, int dblen, int siglen)
{
#ifdef USE_OPENSSL
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
#endif

#ifdef USE_GCRYPT
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
		goto out;
	}

	if (gcry_sexp_build(&signature, NULL, "(sig-val (rsa (s %b)))",
			    siglen, db + dblen)) {
		fprintf(stderr, "Failed to build signature S-expression.\n");
		goto out;
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
			goto out;
		}

		ok = gcry_pk_verify(signature, data, rsa) == 0;
	}
#endif

#if defined(USE_OPENSSL) || defined(USE_GCRYPT)
	if (!ok)
		fprintf(stderr, "Database signature verification failed.\n");

out:
	return ok;
#else
	return 1;
#endif
}

static void reg_rule2rd(uint8_t *db, int dblen,
	uint32_t ruleptr, struct ieee80211_reg_rule *rd_reg_rule)
{
	struct regdb_file_reg_rule *rule;
	struct regdb_file_freq_range *freq;
	struct regdb_file_power_rule *power;

	struct ieee80211_freq_range *rd_freq_range = &rd_reg_rule->freq_range;
	struct ieee80211_power_rule *rd_power_rule = &rd_reg_rule->power_rule;

	rule  = crda_get_file_ptr(db, dblen, sizeof(*rule), ruleptr);
	freq  = crda_get_file_ptr(db, dblen, sizeof(*freq), rule->freq_range_ptr);
	power = crda_get_file_ptr(db, dblen, sizeof(*power), rule->power_rule_ptr);

	rd_freq_range->start_freq_khz = ntohl(freq->start_freq);
	rd_freq_range->end_freq_khz = ntohl(freq->end_freq);
	rd_freq_range->max_bandwidth_khz = ntohl(freq->max_bandwidth);

	rd_power_rule->max_antenna_gain = ntohl(power->max_antenna_gain);
	rd_power_rule->max_eirp = ntohl(power->max_eirp);

	rd_reg_rule->flags = ntohl(rule->flags);
}

/* Converts a file regdomain to ieee80211_regdomain, easier to manage */
static struct ieee80211_regdomain *
country2rd(uint8_t *db, int dblen,
	   struct regdb_file_reg_country *country)
{
	struct regdb_file_reg_rules_collection *rcoll;
	struct ieee80211_regdomain *rd;
	int i, num_rules, size_of_rd;

	rcoll = crda_get_file_ptr(db, dblen, sizeof(*rcoll),
				country->reg_collection_ptr);
	num_rules = ntohl(rcoll->reg_rule_num);
	/* re-get pointer with sanity checking for num_rules */
	rcoll = crda_get_file_ptr(db, dblen,
			sizeof(*rcoll) + num_rules * sizeof(uint32_t),
			country->reg_collection_ptr);

	size_of_rd = sizeof(struct ieee80211_regdomain) +
		num_rules * sizeof(struct ieee80211_reg_rule);

	rd = malloc(size_of_rd);
	if (!rd)
		return NULL;

	memset(rd, 0, size_of_rd);

	rd->alpha2[0] = country->alpha2[0];
	rd->alpha2[1] = country->alpha2[1];
	rd->dfs_region = country->creqs & 0x3;
	rd->n_reg_rules = num_rules;

	for (i = 0; i < num_rules; i++) {
		reg_rule2rd(db, dblen, rcoll->reg_rule_ptrs[i],
			&rd->reg_rules[i]);
	}

	return rd;
}

struct ieee80211_regdomain *
reglib_get_rd_idx(unsigned int idx, const char *file)
{
	int fd;
	struct stat stat;
	uint8_t *db;
	struct regdb_file_header *header;
	struct regdb_file_reg_country *countries;
	int dblen, siglen, num_countries;
	struct ieee80211_regdomain *rd = NULL;
	struct regdb_file_reg_country *country;

	fd = open(file, O_RDONLY);

	if (fd < 0)
		return NULL;

	if (fstat(fd, &stat))
		return NULL;

	dblen = stat.st_size;

	db = mmap(NULL, dblen, PROT_READ, MAP_PRIVATE, fd, 0);
	if (db == MAP_FAILED)
		return NULL;

	header = crda_get_file_ptr(db, dblen, sizeof(*header), 0);

	if (ntohl(header->magic) != REGDB_MAGIC)
		return NULL;

	if (ntohl(header->version) != REGDB_VERSION)
		return NULL;

	siglen = ntohl(header->signature_length);
	/* adjust dblen so later sanity checks don't run into the signature */
	dblen -= siglen;

	if (dblen <= (int)sizeof(*header))
		return NULL;

	/* verify signature */
	if (!crda_verify_db_signature(db, dblen, siglen))
		return NULL;

	num_countries = ntohl(header->reg_country_num);
	countries = crda_get_file_ptr(db, dblen,
			sizeof(struct regdb_file_reg_country) * num_countries,
			header->reg_country_ptr);

	if (idx >= num_countries)
		return NULL;

	country = countries + idx;

	rd = country2rd(db, dblen, country);
	if (!rd)
		return NULL;

	return rd;
}

struct ieee80211_regdomain *
reglib_get_rd_alpha2(const char *alpha2, const char *file)
{
	int fd;
	struct stat stat;
	uint8_t *db;
	struct regdb_file_header *header;
	struct regdb_file_reg_country *countries;
	int dblen, siglen, num_countries;
	struct ieee80211_regdomain *rd = NULL;
	struct regdb_file_reg_country *country;
	unsigned int i;
	bool found_country = false;

	fd = open(file, O_RDONLY);

	if (fd < 0)
		return NULL;

	if (fstat(fd, &stat))
		return NULL;

	dblen = stat.st_size;

	db = mmap(NULL, dblen, PROT_READ, MAP_PRIVATE, fd, 0);
	if (db == MAP_FAILED)
		return NULL;

	header = crda_get_file_ptr(db, dblen, sizeof(*header), 0);

	if (ntohl(header->magic) != REGDB_MAGIC)
		return NULL;

	if (ntohl(header->version) != REGDB_VERSION)
		return NULL;

	siglen = ntohl(header->signature_length);
	/* adjust dblen so later sanity checks don't run into the signature */
	dblen -= siglen;

	if (dblen <= (int)sizeof(*header))
		return NULL;

	/* verify signature */
	if (!crda_verify_db_signature(db, dblen, siglen))
		return NULL;

	num_countries = ntohl(header->reg_country_num);
	countries = crda_get_file_ptr(db, dblen,
			sizeof(struct regdb_file_reg_country) * num_countries,
			header->reg_country_ptr);

	for (i = 0; i < num_countries; i++) {
		country = countries + i;
		if (memcmp(country->alpha2, alpha2, 2) == 0) {
			found_country = 1;
			break;
		}
	}

	if (!found_country)
		goto out;

	rd = country2rd(db, dblen, country);
	if (!rd)
		goto out;

out:
	close(fd);
	return rd;
}
