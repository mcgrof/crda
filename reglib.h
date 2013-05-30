#ifndef REG_LIB_H
#define REG_LIB_H

#include <stdlib.h>
#include <stdint.h>

/* Common regulatory structures, functions and helpers */

/* This matches the kernel's data structures */
struct ieee80211_freq_range {
	uint32_t start_freq_khz;
	uint32_t end_freq_khz;
	uint32_t max_bandwidth_khz;
};

struct ieee80211_power_rule {
	uint32_t max_antenna_gain;
	uint32_t max_eirp;
};

struct ieee80211_reg_rule {
	struct ieee80211_freq_range freq_range;
	struct ieee80211_power_rule power_rule;
	uint32_t flags;
};

struct ieee80211_regdomain {
	uint32_t n_reg_rules;
	char alpha2[2];
	uint8_t dfs_region;
	struct ieee80211_reg_rule reg_rules[];
};

static inline int reglib_is_world_regdom(const char *alpha2)
{
	if (alpha2[0] == '0' && alpha2[1] == '0')
		return 1;
	return 0;
}

static inline int reglib_isalpha_upper(char letter)
{
	if (letter >= 'A' && letter <= 'Z')
		return 1;
	return 0;
}

static inline int is_alpha2(const char *alpha2)
{
	if (reglib_isalpha_upper(alpha2[0]) && reglib_isalpha_upper(alpha2[1]))
		return 1;
	return 0;
}

static inline int is_valid_regdom(const char *alpha2)
{
	if (!is_alpha2(alpha2) && !reglib_is_world_regdom(alpha2))
		return 0;

	return 1;
}

static inline uint32_t max(uint32_t a, uint32_t b)
{
	return (a > b) ? a : b;
}

static inline uint32_t min(uint32_t a, uint32_t b)
{
	return (a > b) ? b : a;
}

void *reglib_get_file_ptr(uint8_t *db, int dblen, int structlen, uint32_t ptr);
int reglib_verify_db_signature(uint8_t *db, int dblen, int siglen);

const struct ieee80211_regdomain *
reglib_get_rd_idx(unsigned int idx, const char *file);

#define reglib_for_each_country(__rd, __idx, __file)			\
	for (__rd = reglib_get_rd_idx(__idx, __file);		\
	     __rd != NULL;						\
	     __rd = reglib_get_rd_idx(++__idx, __file))		\

const struct ieee80211_regdomain *
reglib_get_rd_alpha2(const char *alpha2, const char *file);

/* reg helpers */
void reglib_print_regdom(const struct ieee80211_regdomain *rd);
struct ieee80211_regdomain *
reglib_intersect_rds(const struct ieee80211_regdomain *rd1,
		     const struct ieee80211_regdomain *rd2);

#endif
