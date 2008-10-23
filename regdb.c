#ifdef USE_OPENSSL
#include <stdio.h>
#include <openssl/objects.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>
#endif

#ifdef USE_GCRYPT
#include <stdio.h>
#include <gcrypt.h>
#endif

#include "regdb.h"

#ifdef USE_OPENSSL
#include "keys-ssl.c"
#endif

#ifdef USE_GCRYPT
#include "keys-gcrypt.c"
#endif

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
				keys[0].e, keys[0].len_e, NULL) ||
		    gcry_mpi_scan(&mpi_n, GCRYMPI_FMT_USG,
				keys[0].n, keys[0].len_n, NULL)) {
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
