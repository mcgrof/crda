#include <stdlib.h>
#include <linux/types.h>

/* Common regulatory functions and helpers */

void *crda_get_file_ptr(__u8 *db, int dblen, int structlen, __be32 ptr);
int crda_verify_db_signature(__u8 *db, int dblen, int siglen);

static inline int is_world_regdom(const char *alpha2)
{
	if (alpha2[0] == '0' && alpha2[1] == '0')
		return 1;
	return 0;
}

static inline int isalpha_upper(char letter)
{
	if (letter >= 'A' && letter <= 'Z')
		return 1;
	return 0;
}

static inline int is_alpha2(const char *alpha2)
{
	if (isalpha_upper(alpha2[0]) && isalpha_upper(alpha2[1]))
		return 1;
	return 0;
}

/* Avoid stdlib */
static inline int is_len_2(const char *alpha2)
{
        if (alpha2[0] == '\0' || (alpha2[1] == '\0'))
                return 0;
        if (alpha2[2] == '\0')
                return 1;
        return 0;
}

static inline int is_valid_regdom(const char *alpha2)
{
	if (!is_len_2(alpha2))
		return 0;

	if (!is_alpha2(alpha2) && !is_world_regdom(alpha2))
		return 0;

	return 1;
}
