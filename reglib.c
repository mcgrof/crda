#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include "reglib.h"

#ifdef USE_OPENSSL
#include <openssl/objects.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>
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

void *crda_get_file_ptr(__u8 *db, int dblen, int structlen, __be32 ptr)
{
	__u32 p = ntohl(ptr);

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
int crda_verify_db_signature(__u8 *db, int dblen, int siglen)
{
#ifdef USE_OPENSSL
	RSA *rsa;
	__u8 hash[SHA_DIGEST_LENGTH];
	unsigned int i;
	int ok = 0;

	rsa = RSA_new();
	if (!rsa) {
		fprintf(stderr, "Failed to create RSA key.\n");
		goto out;
	}

	if (SHA1(db, dblen, hash) != hash) {
		fprintf(stderr, "Failed to calculate SHA1 sum.\n");
		RSA_free(rsa);
		goto out;
	}

	for (i = 0; (i < sizeof(keys)/sizeof(keys[0])) && (!ok); i++) {
		rsa->e = &keys[i].e;
		rsa->n = &keys[i].n;

		if (RSA_size(rsa) != siglen)
			continue;

		ok = RSA_verify(NID_sha1, hash, SHA_DIGEST_LENGTH,
				db + dblen, siglen, rsa) == 1;
	}

	rsa->e = NULL;
	rsa->n = NULL;
	RSA_free(rsa);
#endif

#ifdef USE_GCRYPT
	gcry_mpi_t mpi_e, mpi_n;
	gcry_sexp_t rsa, signature, data;
	__u8 hash[20];
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

static void reg_rule2rd(__u8 *db, int dblen,
	__be32 ruleptr, struct ieee80211_reg_rule *rd_reg_rule)
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
struct ieee80211_regdomain *country2rd(__u8 *db, int dblen,
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
			sizeof(*rcoll) + num_rules * sizeof(__be32),
			country->reg_collection_ptr);

	size_of_rd = sizeof(struct ieee80211_regdomain) +
		num_rules * sizeof(struct ieee80211_reg_rule);

	rd = malloc(size_of_rd);
	if (!rd)
		return NULL;

	memset(rd, 0, size_of_rd);

	rd->alpha2[0] = country->alpha2[0];
	rd->alpha2[1] = country->alpha2[1];
	rd->n_reg_rules = num_rules;

	for (i = 0; i < num_rules; i++) {
		reg_rule2rd(db, dblen, rcoll->reg_rule_ptrs[i],
			&rd->reg_rules[i]);
	}

	return rd;
}

static void print_reg_rule(struct ieee80211_reg_rule *rule)
{
	struct ieee80211_freq_range *freq;
	struct ieee80211_power_rule *power;

	freq  = &rule->freq_range;
	power = &rule->power_rule;

	printf("\t(%.3f - %.3f @ %.3f), ",
	       ((float)(freq->start_freq_khz))/1000.0,
	       ((float)(freq->end_freq_khz))/1000.0,
	       ((float)(freq->max_bandwidth_khz))/1000.0);

	printf("(");

	if (power->max_antenna_gain)
		printf("%.2f, ", ((float)(power->max_antenna_gain)/100.0));
	else
		printf("N/A, ");

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
	if (rule->flags & RRF_PASSIVE_SCAN)
		printf(", PASSIVE-SCAN");
	if (rule->flags & RRF_NO_IBSS)
		printf(", NO-IBSS");

	printf("\n");
}

void print_regdom(struct ieee80211_regdomain *rd)
{
	unsigned int i;
	printf("country %.2s:\n", rd->alpha2);
	for (i = 0; i < rd->n_reg_rules; i++)
		print_reg_rule(&rd->reg_rules[i]);
	printf("\n");
}

