
/* This is the DB schema of the CRDA database */

create database if not exists regulatory;
use regulatory;

/* Each regulatory rule defined has a set of frequency ranges with
 * an attached power rule. */
drop table if exists reg_rule;
CREATE TABLE if not exists reg_rule (
        reg_rule_id	int(11) UNSIGNED NOT NULL AUTO_INCREMENT,
        freq_range_id	int(11) UNSIGNED not null default '0',
        power_rule_id	int(11) UNSIGNED not null default '0',
        PRIMARY KEY(reg_rule_id)
);

/* Frequency ranges */
drop table if exists freq_range;
CREATE TABLE if not exists freq_range (
        freq_range_id		int(11) UNSIGNED NOT NULL AUTO_INCREMENT,
        start_freq_khz		int(11) UNSIGNED not null default '0',
        end_freq_khz		int(11) UNSIGNED not null default '0',
        max_bandwidth_khz	int(11) UNSIGNED not null default '0',
        modulation_cap		int(11) UNSIGNED not null default '0',
        misc_restrictions	int(11) UNSIGNED not null default '0',
        PRIMARY KEY(freq_range_id)
);

/* Power rule. Each power rule can be attached to a frequency range.
 * We use mili to avoid floating point in the kernel.
 * EIRP and IR	is given in mili Bells with respect to 1 mili Watt, so mbm
 * Antenna gain	is given in mili Bells with respect to the isotropic, so mbi.
 *
 * Note: a dipole antenna has a gain of 2.14 dBi, so 2140 mBi)
 */
drop table if exists power_rule;
CREATE TABLE if not exists power_rule (
        power_rule_id		int(11) UNSIGNED NOT NULL AUTO_INCREMENT,
        environment_cap		varchar(255) not null,
        max_antenna_gain_mbi	int(11) UNSIGNED not null default '0',
        max_ir_ptmp_mbm	int(11) UNSIGNED not null default '0',
        max_ir_ptp_mbm		int(11) UNSIGNED not null default '0',
        max_eirp_pmtp_mbm	int(11) UNSIGNED not null default '0',
        max_eirp_ptp_mbm	int(11) UNSIGNED not null default '0',
        PRIMARY KEY(power_rule_id)
);

/* Each collection has a list of rules attached it. The reg_collection_id is what we use
 * to group together a bunch of rules into a group. */
drop table if exists reg_rules_collection;
CREATE TABLE if not exists reg_rules_collection (
        entry_id		int(11) UNSIGNED NOT NULL AUTO_INCREMENT,
	reg_collection_id	int(11) UNSIGNED not null default '0',
        reg_rule_id		int(11) UNSIGNED not null default '0',
        PRIMARY KEY(entry_id)
);

/* ISO3166-alpha2 <--> regulatory collection id mapping. Each country can have
 * more than one collection id. This allows for large flexibility on how
 * we can group together in collections rule ids. Right now we group together
 * common rules into bands groups (2 GHz and 5 GHz for now) */
drop table if exists reg_country;
CREATE TABLE if not exists reg_country (
        reg_country_id		int(11) UNSIGNED NOT NULL AUTO_INCREMENT,
        alpha2			varchar(2) not null,
        reg_collection_id	int(11) UNSIGNED not null default '0',
        PRIMARY KEY(reg_country_id)
);

commit;
