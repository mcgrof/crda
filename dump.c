#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h> /* ntohl */

#include "regdb.h"
#include "reglib.h"

static void print_reg_rule(__u8 *db, int dblen, __be32 ruleptr)
{
	struct regdb_file_reg_rule *rule;
	struct regdb_file_freq_range *freq;
	struct regdb_file_power_rule *power;
	__u32 flags;

	rule  = crda_get_file_ptr(db, dblen, sizeof(*rule), ruleptr);
	freq  = crda_get_file_ptr(db, dblen, sizeof(*freq), rule->freq_range_ptr);
	power = crda_get_file_ptr(db, dblen, sizeof(*power), rule->power_rule_ptr);

	printf("\t(%.3f - %.3f @ %.3f), ",
	       ((float)ntohl(freq->start_freq))/1000.0,
	       ((float)ntohl(freq->end_freq))/1000.0,
	       ((float)ntohl(freq->max_bandwidth))/1000.0);

	printf("(");

	if (power->max_antenna_gain)
		printf("%.2f, ", ((float)ntohl(power->max_antenna_gain)/100.0));
	else
		printf("N/A, ");

	if (power->max_eirp)
		printf("%.2f)", ((float)ntohl(power->max_eirp)/100.0));
	else
		printf("N/A)");

	flags = ntohl(rule->flags);
	if (flags & RRF_NO_OFDM)
		printf(", NO-OFDM");
	if (flags & RRF_NO_CCK)
		printf(", NO-CCK");
	if (flags & RRF_NO_INDOOR)
		printf(", NO-INDOOR");
	if (flags & RRF_NO_OUTDOOR)
		printf(", NO-OUTDOOR");
	if (flags & RRF_DFS)
		printf(", DFS");
	if (flags & RRF_PTP_ONLY)
		printf(", PTP-ONLY");
	if (flags & RRF_PTMP_ONLY)
		printf(", PTMP-ONLY");
	if (flags & RRF_PASSIVE_SCAN)
		printf(", PASSIVE-SCAN");
	if (flags & RRF_NO_IBSS)
		printf(", NO-IBSS");

	printf("\n");
}

int main(int argc, char **argv)
{
	int fd;
	struct stat stat;
	__u8 *db;
	struct regdb_file_header *header;
	struct regdb_file_reg_country *countries;
	int dblen, siglen, num_countries, i, j;

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

	header = crda_get_file_ptr(db, dblen, sizeof(*header), 0);

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

	if (dblen <= (int)sizeof(*header)) {
		fprintf(stderr, "Invalid signature length %d\n", siglen);
		return 2;
	}

	/* verify signature */
	if (!crda_verify_db_signature(db, dblen, siglen))
		return -EINVAL;

	num_countries = ntohl(header->reg_country_num);
	countries = crda_get_file_ptr(db, dblen,
			sizeof(struct regdb_file_reg_country) * num_countries,
			header->reg_country_ptr);

	for (i = 0; i < num_countries; i++) {
		struct regdb_file_reg_rules_collection *rcoll;
		struct regdb_file_reg_country *country = countries + i;
		int num_rules;

		printf("country %.2s:\n", country->alpha2);
		rcoll = crda_get_file_ptr(db, dblen, sizeof(*rcoll),
					country->reg_collection_ptr);
		num_rules = ntohl(rcoll->reg_rule_num);
		/* re-get pointer with sanity checking for num_rules */
		rcoll = crda_get_file_ptr(db, dblen,
				sizeof(*rcoll) + num_rules * sizeof(__be32),
				country->reg_collection_ptr);
		for (j = 0; j < num_rules; j++)
			print_reg_rule(db, dblen, rcoll->reg_rule_ptrs[j]);
		printf("\n");
	}

	return 0;
}
