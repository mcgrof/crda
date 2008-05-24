#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h> /* ntohl */

#include "regdb.h"

#ifdef VERIFY_SIGNATURE

#ifdef USE_OPENSSL
#include <openssl/objects.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>

#include "keys-ssl.c"
#endif

#ifdef USE_GCRYPT
#include <gcrypt.h>

#include "keys-gcrypt.c"
#endif

#endif

static void *get_file_ptr(__u8 *db, int dblen, int structlen, __be32 ptr)
{
	__u32 p = ntohl(ptr);

	if (p > dblen - structlen) {
		fprintf(stderr, "Invalid database file, bad pointer!\n");
		exit(3);
	}

	return (void *)(db + p);
}

static void print_reg_rule(__u8 *db, int dblen, __be32 ruleptr)
{
	struct regdb_file_reg_rule *rule;
	struct regdb_file_freq_range *freq;
	struct regdb_file_power_rule *power;
	char *ofdm = "", *cck = "", env[3] = { 'O', 'I', '\0' };

	rule = get_file_ptr(db, dblen, sizeof(*rule), ruleptr);
	freq = get_file_ptr(db, dblen, sizeof(*freq), rule->freq_range_ptr);
	power = get_file_ptr(db, dblen, sizeof(*power), rule->power_rule_ptr);

	if (ntohl(freq->modulation_cap) & 2)
		ofdm = ", OFDM";
	if (ntohl(freq->modulation_cap) & 1)
		cck = ", CCK";

	printf("\t(%.3f - %.3f @ %.3f%s%s), ",
	       (float)ntohl(freq->start_freq)/1000.0,
	       (float)ntohl(freq->end_freq)/1000.0,
	       (float)ntohl(freq->max_bandwidth)/1000.0,
	       cck, ofdm);

	if (power->environment_cap != ' ') {
		env[0] = power->environment_cap;
		env[1] = '\0';
	}

	printf("(%s, %.3f, %.3f, %.3f, %.3f, %.3f)\n",
	       env,
	       (float)ntohl(power->max_antenna_gain/100.0),
	       (float)ntohl(power->max_ir_ptmp/100.0),
	       (float)ntohl(power->max_ir_ptp/100.0),
	       (float)ntohl(power->max_eirp_pmtp/100.0),
	       (float)ntohl(power->max_eirp_ptp)/100.0);
}

int main(int argc, char **argv)
{
	int fd;
	struct stat stat;
	__u8 *db;
	struct regdb_file_header *header;
	struct regdb_file_reg_country *countries;
	int dblen, siglen, num_countries, i, j;
#ifdef USE_OPENSSL
	RSA *rsa;
	__u8 hash[SHA_DIGEST_LENGTH];
	int ok = 0;
#endif
#ifdef USE_GCRYPT
	gcry_mpi_t mpi_e, mpi_n;
	gcry_sexp_t rsa, signature, data;
	__u8 hash[20];
	int ok = 0;
#endif

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		return 2;
	}

	fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		perror("failed to open db file");
		return 2;
	}

	if (fstat(fd, &stat)) {
		perror("failed to fstat db file");
		return 2;
	}

	dblen = stat.st_size;

	db = mmap(NULL, dblen, PROT_READ, MAP_PRIVATE, fd, 0);
	if (db == MAP_FAILED) {
		perror("failed to mmap db file");
		return 2;
	}

	header = get_file_ptr(db, dblen, sizeof(*header), 0);

	if (ntohl(header->magic) != REGDB_MAGIC) {
		fprintf(stderr, "Invalid database magic\n");
		return 2;
	}

	if (ntohl(header->version) != REGDB_VERSION) {
		fprintf(stderr, "Invalid database version\n");
		return 2;
	}

	siglen = ntohl(header->signature_length);
	/* adjust dblen so later sanity checks don't run into the signature */
	dblen -= siglen;

	if (dblen <= sizeof(*header)) {
		fprintf(stderr, "Invalid signature length %d\n", siglen);
		return 2;
	}

	/* verify signature */
#ifdef USE_OPENSSL
	rsa = RSA_new();
	if (!rsa) {
		fprintf(stderr, "Failed to create RSA key\n");
		return 2;
	}

	if (SHA1(db, dblen, hash) != hash) {
		fprintf(stderr, "Failed to calculate SHA sum\n");
		return 2;
	}

	for (i = 0; i < sizeof(keys)/sizeof(keys[0]); i++) {
		rsa->e = &keys[i].e;
		rsa->n = &keys[i].n;

		if (RSA_size(rsa) != siglen)
			continue;

		ok = RSA_verify(NID_sha1, hash, SHA_DIGEST_LENGTH,
				db + dblen, siglen, rsa) == 1;
		if (ok)
			break;
	}

	if (!ok) {
		fprintf(stderr, "Database signature wrong\n");
		return 2;
	}

	rsa->e = NULL;
	rsa->n = NULL;
	RSA_free(rsa);

	BN_print_fp(stdout, &keys[0].n);

	return 0;
#endif

#ifdef USE_GCRYPT
	/* hash the db */
	gcry_md_hash_buffer(GCRY_MD_SHA1, hash, db, dblen);

	if (gcry_sexp_build(&data, NULL, "(data (flags pkcs1) (hash sha1 %b))",
			    20, hash)) {
		fprintf(stderr, "failed to build data expression\n");
		return 2;
	}

	if (gcry_sexp_build(&signature, NULL, "(sig-val (rsa (s %b)))",
			    siglen, db + dblen)) {
		fprintf(stderr, "failed to build signature expression\n");
		return 2;
	}

	for (i = 0; i < sizeof(keys)/sizeof(keys[0]); i++) {
		if (gcry_mpi_scan(&mpi_e, GCRYMPI_FMT_USG,
				  keys[0].e, keys[0].len_e, NULL) ||
		    gcry_mpi_scan(&mpi_n, GCRYMPI_FMT_USG,
		    		  keys[0].n, keys[0].len_n, NULL)) {
			fprintf(stderr, "failed to convert numbers\n");
			return 2;
		}

		if (gcry_sexp_build(&rsa, NULL,
				    "(public-key (rsa (n %m) (e %m)))",
				    mpi_n, mpi_e)) {
			fprintf(stderr, "failed to build rsa key\n");
			return 2;
		}

		if (!gcry_pk_verify(signature, data, rsa)) {
			ok = 1;
			break;
		}
	}

	if (!ok) {
		fprintf(stderr, "Database signature wrong\n");
		return 2;
	}

#endif

	num_countries = ntohl(header->reg_country_num);
	countries = get_file_ptr(db, dblen,
				 sizeof(struct regdb_file_reg_country) * num_countries,
				 header->reg_country_ptr);

	for (i = 0; i < num_countries; i++) {
		struct regdb_file_reg_rules_collection *rcoll;
		struct regdb_file_reg_country *country = countries + i;
		int num_rules;

		printf("country %.2s:\n", country->alpha2);
		rcoll = get_file_ptr(db, dblen, sizeof(*rcoll), country->reg_collection_ptr);
		num_rules = ntohl(rcoll->reg_rule_num);
		/* re-get pointer with sanity checking for num_rules */
		rcoll = get_file_ptr(db, dblen,
				     sizeof(*rcoll) + num_rules * sizeof(__be32),
				     country->reg_collection_ptr);
		for (j = 0; j < num_rules; j++)
			print_reg_rule(db, dblen, rcoll->reg_rule_ptrs[j]);
		printf("\n");
	}

	return 0;
}
