#include <stdio.h>
#include <errno.h>
#include "reglib.h"

static int reglib_regdbdump(char *regdb_file)
{
	const struct ieee80211_regdomain *rd = NULL;
	unsigned int idx = 0;

	reglib_for_each_country(rd, idx, regdb_file) {
		reglib_print_regdom(rd);
		free((struct ieee80211_regdomain *) rd);
	}

	if (!idx) {
		fprintf(stderr, "Invalid or empty regulatory file, note: "
		       "a binary regulatory file should be used.\n");
		return -EINVAL;
	}

	return 0;
}

int main(int argc, char **argv)
{
	int r;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <regulatory-binary-file>\n", argv[0]);
		return 2;
	}

	r = reglib_regdbdump(argv[1]);

	return r;
}
