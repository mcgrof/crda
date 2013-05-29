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
	struct ieee80211_regdomain *prev_world = NULL, *world = NULL;
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

		if (!prev_world) {
			prev_world = (struct ieee80211_regdomain *) rd;
			continue;
		}

		if (world) {
			free(prev_world);
			prev_world = (struct ieee80211_regdomain *) world;
		}

		world = regdom_intersect(prev_world, rd);
		if (!world) {
			free(prev_world);
			free((struct ieee80211_regdomain *) rd);
			fprintf(stderr, "Intersection not possible\n");
			return -ENOENT;
		}

		if (intersected)
			/* Use UTF-8 Intersection symbol ? (0xE2,0x88,0xA9) :) */
			printf("WW (%d) intersect %c%c (%d) ==> %d rules\n",
				prev_world->n_reg_rules,
				rd->alpha2[0],
				rd->alpha2[1],
				rd->n_reg_rules,
				world->n_reg_rules);
		else
			printf("%c%c (%d) intersect %c%c (%d) ==> %d rules\n",
				prev_world->alpha2[0],
				prev_world->alpha2[1],
				prev_world->n_reg_rules,
				rd->alpha2[0],
				rd->alpha2[1],
				rd->n_reg_rules,
				world->n_reg_rules);
		intersected++;
		free((struct ieee80211_regdomain *) rd);
	}

	if (!idx) {
		printf("Invalid or empty regulatory file, note: "
		       "a binary regulatory file should be used.\n");
		return -EINVAL;
	}

	if (intersected > 1)
		printf("%d regulatory domains intersected\n", intersected);
	else {
		world = prev_world;
		prev_world = NULL;
		printf("No intersections completed\n");
		if (idx > 1) {
			printf("Since more than one regulatory domain is "
			       "present and no intersections were possible "
			       "no globally allowed spectrum is possible so "
			       "consider enabling passive scan flags\n");
			free(world);
			return r;
		}
	}

	if (prev_world)
		free(prev_world);

	/* Tada! */
	printf("== World regulatory domain: ==\n");
	print_regdom(world);

	free(world);
	return r;
}
