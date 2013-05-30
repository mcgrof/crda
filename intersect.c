#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h> /* ntohl */
#include <string.h>

#include "reglib.h"

/* Intersects regulatory domains, this will skip any regulatory marked with
 * an alpha2 of '00', which is used to indicate a regulatory domain */

int main(int argc, char **argv)
{
	int r = 0;
	const struct ieee80211_regdomain *rd;
	struct ieee80211_regdomain *prev_rd_intsct = NULL, *rd_intsct = NULL;
	int intersected = 0;
	unsigned int idx = 0;

	if (argc != 2) {
		fprintf(stderr, "You must specify a file\n");
		return -EINVAL;
	}

	/* We intersect only when we have to rd structures ready */
	reglib_for_each_country(rd, idx, argv[1]) {
		if (is_world_regdom((const char *) rd->alpha2)) {
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

		rd_intsct = regdom_intersect(prev_rd_intsct, rd);
		if (!rd_intsct) {
			free(prev_rd_intsct);
			free((struct ieee80211_regdomain *) rd);
			return -ENOENT;
		}

		intersected++;
		free((struct ieee80211_regdomain *) rd);
	}

	if (!idx)
		return -EINVAL;

	if (intersected <= 0) {
		rd_intsct = prev_rd_intsct;
		prev_rd_intsct = NULL;
		if (idx > 1) {
			r = -ENOENT;
			free(rd_intsct);
			return r;
		}
	}

	if (prev_rd_intsct)
		free(prev_rd_intsct);

	/* Tada! */
	printf("== World regulatory domain: ==\n");
	print_regdom(rd_intsct);

	free(rd_intsct);
	return r;
}
