#!/usr/bin/env python

from cStringIO import StringIO
import struct
import sha
from dbparse import DBParser, create_rules, create_collections

MAGIC = 0x52474442
VERSION = 19

p = DBParser()
bands, power, countries = p.parse(file('db.txt'))
rules = create_rules(countries)
collections = create_collections(countries)

print """
use regulatory;

/* This is the DB data for the CRDA database */

/* Each regulatory rule defined has a set of frequency ranges with
 * an attached power rule. */
drop table if exists reg_rule;
CREATE TABLE reg_rule (
        reg_rule_id	INTEGER PRIMARY KEY NOT NULL,
        freq_range_id	INTEGER NOT NULL default '0',
        power_rule_id	INTEGER NOT NULL default '0'
);

/* Frequency ranges */
drop table if exists freq_range;
CREATE TABLE freq_range (
        freq_range_id		INTEGER PRIMARY KEY NOT NULL,
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
        power_rule_id		INTEGER PRIMARY KEY NOT NULL,
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
        entry_id		INTEGER PRIMARY KEY NOT NULL,
	reg_collection_id	INTEGER NOT NULL default '0',
        reg_rule_id		INTEGER NOT NULL default '0'
);

/* ISO3166-alpha2 <--> regulatory collection id mapping. Each country can have
 * more than one collection id. This allows for large flexibility on how
 * we can group together in collections rule ids. Right now we group together
 * common rules into bands groups (2 GHz and 5 GHz for now) */
drop table if exists reg_country;
CREATE TABLE reg_country (
        reg_country_id		INTEGER PRIMARY KEY NOT NULL,
        alpha2			char(2) NOT NULL,
        reg_collection_id	INTEGER NOT NULL default '0'
);
"""

power_rules = {}
power_rule = 0
for power_rule_id, pr in power.iteritems():
    power_rule += 1
    environ = pr[0]
    pr = [int(v * 1000) for v in pr[1:]]
    power_rules[power_rule_id] = power_rule
    print 'INSERT INTO power_rule (power_rule_id, environment_cap, max_antenna_gain_mbi, max_ir_ptmp_mbm, max_ir_ptp_mbm, max_eirp_pmtp_mbm, max_eirp_ptp_mbm) VALUES',
    print "(%d, '%s', %d, %d, %d, %d, %d);" % (power_rule, environ, pr[0], pr[1], pr[2], pr[3], pr[4])

freq_ranges = {}
freq_range = 0
for freq_range_id, fr in bands.iteritems():
    freq_range += 1
    freq_ranges[freq_range_id] = freq_range
    fl = fr[3]
    fr = [int(f * 1000) for f in fr[:3]]
    print 'INSERT INTO freq_range (freq_range_id, start_freq_khz, end_freq_khz, max_bandwidth_khz, modulation_cap) VALUES',
    print '(%d, %d, %d, %d, %d);' % (freq_range, fr[0], fr[1], fr[2], fl)


reg_rules = {}
reg_rule = 0
for freq_range_id, power_rule_id in rules:
    reg_rule += 1
    reg_rules[(freq_range_id, power_rule_id)] = reg_rule
    print 'INSERT INTO reg_rule (reg_rule_id, freq_range_id, power_rule_id) VALUES',
    print '(%d, %d, %d);' % (reg_rule, freq_ranges[freq_range_id], power_rules[power_rule_id])


reg_rules_collections = {}
reg_rule_coll = 0
reg_rule_entry = 0
for coll in collections:
    reg_rule_coll += 1
    reg_rules_collections[coll] = reg_rule_coll
    for c in coll:
        reg_rule_entry += 1
        print 'INSERT INTO reg_rules_collection (entry_id, reg_collection_id, reg_rule_id) VALUES',
        print '(%d, %d, %d);' % (reg_rule_entry, reg_rule_coll, reg_rules[c])

country_id = 0
for alpha2, coll in countries.iteritems():
    country_id += 1
    print 'INSERT INTO reg_country (reg_country_id, alpha2, reg_collection_id) VALUES',
    print "(%d, '%s', %d);" % (country_id, alpha2, reg_rules_collections[coll])
