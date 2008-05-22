#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h> /* ntohl */

#include "regdb.h"

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
	int dblen, num_countries, i, j;

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
