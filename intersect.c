#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h> /* ntohl */
#include <string.h>

#include "reglib.h"

/**
 * reglib_intersect_regdb - intersects a regulatory database
 *
 * @regdb_file: the regulatory database to intersect
 *
 * Goes through an entire regulatory database and intersects all regulatory
 * domains. This will skip any regulatory marked with an alpha2 of '00', which
 * is used to indicate a world regulatory domain. If intersection is able
 * to find rules that fit all regulatory domains it return a regulatory
 * domain with such rules otherwise it returns NULL.
 */
const struct ieee80211_regdomain *reglib_intersect_regdb(char *regdb_file)
{
	const struct ieee80211_regdomain *rd;
	struct ieee80211_regdomain *prev_rd_intsct = NULL, *rd_intsct = NULL;
	int intersected = 0;
	unsigned int idx = 0;

	reglib_for_each_country(rd, idx, regdb_file) {
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

int main(int argc, char **argv)
{
	const struct ieee80211_regdomain *rd;

	if (argc != 2) {
		fprintf(stderr, "You must specify a file\n");
		return -EINVAL;
	}

	rd = reglib_intersect_regdb(argv[1]);
	if (!rd) {
		fprintf(stderr, "Intersection not possible\n");
		return -ENOENT;
	}

	reglib_print_regdom(rd);
	free((struct ieee80211_regdomain *) rd);

	return 0;
}
