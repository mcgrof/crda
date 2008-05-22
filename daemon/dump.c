#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h> /* ntohl */

#include "regdb.h"

static void print_reg_rule(__u8 *db, struct regdb_file_reg_rule *rule)
{
	struct regdb_file_freq_range *freq;
	struct regdb_file_power_rule *power;

	freq = (void *)(db + ntohl(rule->freq_range_ptr));
	power = (void *)(db + ntohl(rule->power_rule_ptr));

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
	int num_countries, i, j;

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

	/* XXX: check file length to be at least >= sizeof(struct regdb_file_header) */

	db = mmap(NULL, stat.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (db == MAP_FAILED) {
		perror("failed to mmap db file");
		return 2;
	}

	header = (void *)db;

	if (ntohl(header->magic) != REGDB_MAGIC) {
		fprintf(stderr, "Invalid database magic\n");
		return 2;
	}

	if (ntohl(header->version) != REGDB_VERSION) {
		fprintf(stderr, "Invalid database version\n");
		return 2;
	}

	num_countries = ntohl(header->reg_country_num);
	countries = (void *)(db + ntohl(header->reg_country_ptr));

	/* XXX: check countries pointer to be in file */

	for (i = 0; i < num_countries; i++) {
		struct regdb_file_reg_rules_collection *rcoll;
		struct regdb_file_reg_country *country = countries + i;

		printf("Country %.2s\n", country->alpha2);
		rcoll = (void *)db + ntohl(country->reg_collection_ptr);
		for (j = 0; j < ntohl(rcoll->reg_rule_num); j++)
			print_reg_rule(db, (void *)(db + ntohl(rcoll->reg_rule_ptrs[j])));
	}

	return 0;
}
