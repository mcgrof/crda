#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h> /* ntohl */
#include <string.h>

#include "reglib.h"

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
