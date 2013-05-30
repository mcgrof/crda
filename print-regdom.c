#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include "nl80211.h"

#include "reglib.h"

static const char *dfs_domain_name(enum nl80211_dfs_regions region)
{
	switch (region) {
	case NL80211_DFS_UNSET:
		return "DFS-UNSET";
	case NL80211_DFS_FCC:
		return "DFS-FCC";
	case NL80211_DFS_ETSI:
		return "DFS-ETSI";
	case NL80211_DFS_JP:
		return "DFS-JP";
	default:
		return "DFS-invalid";
	}
}

static void print_reg_rule(const struct ieee80211_reg_rule *rule)
{
	const struct ieee80211_freq_range *freq;
	const struct ieee80211_power_rule *power;

	freq  = &rule->freq_range;
	power = &rule->power_rule;

	printf("\t(%.3f - %.3f @ %.3f), ",
	       ((float)(freq->start_freq_khz))/1000.0,
	       ((float)(freq->end_freq_khz))/1000.0,
	       ((float)(freq->max_bandwidth_khz))/1000.0);

	printf("(");

	if (power->max_antenna_gain)
		printf("%.2f, ", ((float)(power->max_antenna_gain)/100.0));
	else
		printf("N/A, ");

	if (power->max_eirp)
		printf("%.2f)", ((float)(power->max_eirp)/100.0));
	else
		printf("N/A)");

	if (rule->flags & NL80211_RRF_NO_OFDM)
		printf(", NO-OFDM");
	if (rule->flags & NL80211_RRF_NO_CCK)
		printf(", NO-CCK");
	if (rule->flags & NL80211_RRF_NO_INDOOR)
		printf(", NO-INDOOR");
	if (rule->flags & NL80211_RRF_NO_OUTDOOR)
		printf(", NO-OUTDOOR");
	if (rule->flags & NL80211_RRF_DFS)
		printf(", DFS");
	if (rule->flags & NL80211_RRF_PTP_ONLY)
		printf(", PTP-ONLY");
	if (rule->flags & NL80211_RRF_PTMP_ONLY)
		printf(", PTMP-ONLY");
	if (rule->flags & NL80211_RRF_PASSIVE_SCAN)
		printf(", PASSIVE-SCAN");
	if (rule->flags & NL80211_RRF_NO_IBSS)
		printf(", NO-IBSS");

	printf("\n");
}

void reglib_print_regdom(const struct ieee80211_regdomain *rd)
{
	unsigned int i;
	printf("country %.2s: %s\n", rd->alpha2,
	       dfs_domain_name(rd->dfs_region));
	for (i = 0; i < rd->n_reg_rules; i++)
		print_reg_rule(&rd->reg_rules[i]);
	printf("\n");
}
