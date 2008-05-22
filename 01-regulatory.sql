use regulatory;

/* This is the DB schema of the CRDA database */

/* Each regulatory rule defined has a set of frequency ranges with
 * an attached power rule. */
drop table if exists reg_rule;
CREATE TABLE reg_rule (
        reg_rule_id	INTEGER PRIMARY KEY AUTO_INCREMENT NOT NULL,
        freq_range_id	INTEGER NOT NULL default '0',
        power_rule_id	INTEGER NOT NULL default '0'
);

/* Frequency ranges */
drop table if exists freq_range;
CREATE TABLE freq_range (
        freq_range_id		INTEGER PRIMARY KEY AUTO_INCREMENT NOT NULL,
        start_freq_khz		INTEGER NOT NULL default '0',
        end_freq_khz		INTEGER NOT NULL default '0',
        max_bandwidth_khz	INTEGER NOT NULL default '0',
        modulation_cap		INTEGER NOT NULL default '0',
        misc_restrictions	INTEGER NOT NULL default '0'
);

/* Power rule. Each power rule can be attached to a frequency range.
 * We use mili to avoid floating point in the kernel.
 * EIRP and IR	is given in mili Bells with respect to 1 mili Watt, so mbm
 * Antenna gain	is given in mili Bells with respect to the isotropic, so mbi.
 *
 * Note: a dipole antenna has a gain of 2.14 dBi, so 2140 mBi)
 */
drop table if exists power_rule;
CREATE TABLE power_rule (
        power_rule_id		INTEGER PRIMARY KEY AUTO_INCREMENT NOT NULL,
        environment_cap		char(1) NOT NULL,
        max_antenna_gain_mbi	INTEGER NOT NULL default '0',
        max_ir_ptmp_mbm		INTEGER NOT NULL default '0',
        max_ir_ptp_mbm		INTEGER NOT NULL default '0',
        max_eirp_pmtp_mbm	INTEGER NOT NULL default '0',
        max_eirp_ptp_mbm	INTEGER NOT NULL default '0'
);

/* Each collection has a list of rules attached it. The reg_collection_id is what we use
 * to group together a bunch of rules into a group. */
drop table if exists reg_rules_collection;
CREATE TABLE reg_rules_collection (
        entry_id		INTEGER PRIMARY KEY AUTO_INCREMENT NOT NULL,
	reg_collection_id	INTEGER NOT NULL default '0',
        reg_rule_id		INTEGER NOT NULL default '0'
);

/* ISO3166-alpha2 <--> regulatory collection id mapping. Each country can have
 * more than one collection id. This allows for large flexibility on how
 * we can group together in collections rule ids. Right now we group together
 * common rules into bands groups (2 GHz and 5 GHz for now) */
drop table if exists reg_country;
CREATE TABLE reg_country (
        reg_country_id		INTEGER PRIMARY KEY AUTO_INCREMENT NOT NULL,
        alpha2			char(2) NOT NULL,
        reg_collection_id	INTEGER NOT NULL default '0'
);
