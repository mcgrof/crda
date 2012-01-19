#include <stdio.h>
#include "reglib.h"

int main(int argc, char **argv)
{
	struct ieee80211_regdomain *rd = NULL;
	unsigned int idx = 0;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		return 2;
	}

	reglib_for_each_country(rd, idx, argv[1]) {
		print_regdom(rd);
		free(rd);
	}

	return 0;
}
