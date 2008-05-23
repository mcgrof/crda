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
/*#define USE_OPENSSL		1*/
#define USE_GCRYPT		1

#ifdef USE_OPENSSL
#include <openssl/objects.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>
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

#ifdef USE_GCRYPT
#include <gcrypt.h>

struct key_params {
	__u8 *e, *n;
	__u32 len_e, len_n;
};

#define KEYS(_e, _n) {			\
	.e = _e, .len_e = sizeof(_e),	\
	.n = _n, .len_n = sizeof(_n),	\
}


/*
 * public key
 * generated using ./scripts/mk-gcrypt-mpi.sh
 */
static __u8 e_1[] = { 0, 1, 0, 1, };

static __u8 n_1[] = {
	0xb8,0xba,0x00,0x78,
	0x1d,0x43,0x51,0xc7,
	0x89,0x65,0xda,0x96,
	0x67,0x93,0x99,0x18,
	0xb5,0x9f,0xb8,0xcf,
	0x4e,0xa1,0x7c,0xbe,
	0x7a,0xd7,0x69,0xaf,
	0x49,0xa0,0xbb,0xaf,
	0x3d,0xa3,0x0a,0xde,
	0xc6,0x2f,0x93,0x17,
	0xb6,0x36,0x2b,0x65,
	0x1f,0x5f,0x2c,0xaa,
	0xf2,0x1a,0x19,0x12,
	0x82,0x2c,0x42,0x5b,
	0xa8,0x90,0x4b,0x63,
	0x5c,0x91,0x8c,0xbe,
	0xff,0xf8,0x1b,0x51,
	0x25,0xea,0x96,0x55,
	0xe0,0xdf,0x59,0xb0,
	0x10,0x4a,0x88,0x4f,
	0x9a,0x1f,0xe8,0x02,
	0x6b,0x48,0x98,0xfa,
	0xe7,0xc8,0xa3,0x92,
	0xa2,0x20,0xe1,0x5f,
	0xb4,0x43,0x57,0x90,
	0x0d,0xc4,0x50,0xf3,
	0xa9,0x56,0x26,0x50,
	0xd8,0xe6,0xd1,0x15,
	0x00,0x2a,0x4f,0xcc,
	0x95,0x2d,0x00,0x39,
	0xcb,0x23,0x07,0x27,
	0x86,0xff,0x93,0xe1,
	0xb1,0x66,0x02,0xc7,
	0xfa,0x06,0x9e,0x17,
	0x65,0x1d,0x4b,0xce,
	0x45,0x02,0x82,0x23,
	0x93,0x14,0x51,0x01,
	0x32,0xf8,0xb6,0xa0,
	0x6e,0x4b,0x68,0x56,
	0xd5,0xa8,0xda,0x9c,
	0x51,0x1c,0x44,0x1c,
	0x99,0x98,0x74,0xc2,
	0x5c,0xbf,0xd6,0xc3,
	0x6b,0x8e,0x4c,0x6e,
	0x91,0x3d,0x38,0xe3,
	0x27,0xff,0x86,0xeb,
	0x63,0x13,0x3e,0xa0,
	0xf0,0x9a,0x0b,0x0b,
	0xc8,0x6a,0x42,0xa1,
	0xc3,0x97,0x04,0x20,
	0x50,0x1f,0x94,0x19,
	0x06,0x7f,0x3c,0x4c,
	0x63,0xdc,0xde,0x0a,
	0xde,0x51,0x84,0x41,
	0xb1,0xd8,0xe9,0xd2,
	0x8b,0xea,0xb5,0xd7,
	0xa2,0x34,0x4f,0xa8,
	0xd3,0x6a,0xdd,0x15,
	0xcc,0xe6,0x7a,0x85,
	0x58,0xda,0x64,0x6a,
	0x9e,0xf6,0xf7,0x46,
	0xc3,0x13,0xca,0x17,
	0xed,0xdb,0x63,0x4f,
	0xee,0x2c,0xc7,0x2d,
};

static struct key_params keys[] = {
	KEYS(e_1, n_1),
};

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
	       (float)ntohl(power->max_antenna_gain/1000.0),
	       (float)ntohl(power->max_ir_ptmp/1000.0),
	       (float)ntohl(power->max_ir_ptp/1000.0),
	       (float)ntohl(power->max_eirp_pmtp/1000.0),
	       (float)ntohl(power->max_eirp_ptp)/1000.0);
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
