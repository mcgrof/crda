#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h> /* ntohl */

#include "regdb.h"

#define VERIFY_SIGNATURE	1

#ifdef VERIFY_SIGNATURE
#define USE_OPENSSL		1

#ifdef USE_OPENSSL
#include <openssl/objects.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>
#endif

/*
 * public key
 * generated using scripts/mk-openssl-bignum.sh
 */
static BN_ULONG e_1[] = { 65537, };

static BN_ULONG n_1[] = {
	0xee2cc72d,
	0xeddb634f,
	0xc313ca17,
	0x9ef6f746,
	0x58da646a,
	0xcce67a85,
	0xd36add15,
	0xa2344fa8,
	0x8beab5d7,
	0xb1d8e9d2,
	0xde518441,
	0x63dcde0a,
	0x067f3c4c,
	0x501f9419,
	0xc3970420,
	0xc86a42a1,
	0xf09a0b0b,
	0x63133ea0,
	0x27ff86eb,
	0x913d38e3,
	0x6b8e4c6e,
	0x5cbfd6c3,
	0x999874c2,
	0x511c441c,
	0xd5a8da9c,
	0x6e4b6856,
	0x32f8b6a0,
	0x93145101,
	0x45028223,
	0x651d4bce,
	0xfa069e17,
	0xb16602c7,
	0x86ff93e1,
	0xcb230727,
	0x952d0039,
	0x002a4fcc,
	0xd8e6d115,
	0xa9562650,
	0x0dc450f3,
	0xb4435790,
	0xa220e15f,
	0xe7c8a392,
	0x6b4898fa,
	0x9a1fe802,
	0x104a884f,
	0xe0df59b0,
	0x25ea9655,
	0xfff81b51,
	0x5c918cbe,
	0xa8904b63,
	0x822c425b,
	0xf21a1912,
	0x1f5f2caa,
	0xb6362b65,
	0xc62f9317,
	0x3da30ade,
	0x49a0bbaf,
	0x7ad769af,
	0x4ea17cbe,
	0xb59fb8cf,
	0x67939918,
	0x8965da96,
	0x1d4351c7,
	0xb8ba0078,
};

struct pubkey {
	struct bignum_st e, n;
};

#define KEY(data)	{			\
	.d = data,				\
	.top = sizeof(data)/sizeof(data[0]),	\
}

#define KEYS(e,n)	{ KEY(e), KEY(n), }

static struct pubkey keys[] = {
	KEYS(e_1, n_1),
};
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

	rule = get_file_ptr(db, dblen, sizeof(*rule), ruleptr);
	freq = get_file_ptr(db, dblen, sizeof(*freq), rule->freq_range_ptr);
	power = get_file_ptr(db, dblen, sizeof(*power), rule->power_rule_ptr);

	printf("\t%d..%d kHz (maxbw: %d kHz, mod: 0x%x, restrict: 0x%x)\n",
	       ntohl(freq->start_freq),
	       ntohl(freq->end_freq),
	       ntohl(freq->max_bandwidth),
	       ntohl(freq->modulation_cap),
	       ntohl(freq->misc_restrictions));

	printf("\t -> env: '%.1s', ag: %d, ir_ptmp: %d, ir_ptp: %d, eirp_pmtp: %d, eirp_ptp: %d\n",
	       &power->environment_cap,
	       ntohl(power->max_antenna_gain),
	       ntohl(power->max_ir_ptmp),
	       ntohl(power->max_ir_ptp),
	       ntohl(power->max_eirp_pmtp),
	       ntohl(power->max_eirp_ptp));
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

#ifdef USE_OPENSSL
	/* verify signature */
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
#endif

	num_countries = ntohl(header->reg_country_num);
	countries = get_file_ptr(db, dblen,
				 sizeof(struct regdb_file_reg_country) * num_countries,
				 header->reg_country_ptr);

	for (i = 0; i < num_countries; i++) {
		struct regdb_file_reg_rules_collection *rcoll;
		struct regdb_file_reg_country *country = countries + i;
		int num_rules;

		printf("Country %.2s\n", country->alpha2);
		rcoll = get_file_ptr(db, dblen, sizeof(*rcoll), country->reg_collection_ptr);
		num_rules = ntohl(rcoll->reg_rule_num);
		/* re-get pointer with sanity checking for num_rules */
		rcoll = get_file_ptr(db, dblen,
				     sizeof(*rcoll) + num_rules * sizeof(__be32),
				     country->reg_collection_ptr);
		for (j = 0; j < num_rules; j++)
			print_reg_rule(db, dblen, rcoll->reg_rule_ptrs[j]);
	}

	return 0;
}
