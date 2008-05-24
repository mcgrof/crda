#!/usr/bin/env python

from cStringIO import StringIO
import struct
import sha
from dbparse import DBParser, create_rules, create_collections

MAGIC = 0x52474442
VERSION = 19

def be32(output, val):
    output.write(struct.pack('>I', val))

class PTR(object):
    def __init__(self, output):
        self._output = output
        self._pos = output.tell()
        be32(output, 0xFFFFFFFF)

    def set(self, val=None):
        if val is None:
            val = self._output.tell()
        self._offset = val
        pos = self._output.tell()
        self._output.seek(self._pos)
        be32(self._output, val)
        self._output.seek(pos)

    def get(self):
        return self._offset

p = DBParser()
bands, power, countries = p.parse(file('db.txt'))
rules = create_rules(countries)
collections = create_collections(countries)

output = StringIO()

# struct regdb_file_header
be32(output, MAGIC)
be32(output, VERSION)
reg_country_ptr = PTR(output)
# add number of countries
be32(output, len(countries))
siglen = PTR(output)

power_rules = {}
for power_rule_id, pr in power.iteritems():
    environ = pr[0]
    pr = [int(v * 100) for v in pr[1:]]
    power_rules[power_rule_id] = output.tell()
    # struct regdb_file_power_rule
    output.write(struct.pack('>cxxxIIIII', str(environ), *pr))

freq_ranges = {}
for freq_range_id, fr in bands.iteritems():
    freq_ranges[freq_range_id] = output.tell()
    fl = fr[3]
    fr = [int(f * 1000) for f in fr[:3]]
    # struct regdb_file_freq_range
    output.write(struct.pack('>IIIII', fr[0], fr[1], fr[2], fl, 0))


reg_rules = {}
for reg_rule in rules:
    freq_range_id, power_rule_id = reg_rule
    reg_rules[reg_rule] = output.tell()
    # struct regdb_file_reg_rule
    output.write(struct.pack('>II', freq_ranges[freq_range_id], power_rules[power_rule_id]))


reg_rules_collections = {}

for coll in collections:
    reg_rules_collections[coll] = output.tell()
    # struct regdb_file_reg_rules_collection
    be32(output, len(coll))
    coll = list(coll)
    coll.sort(cmp=lambda x, y: cmp(bands[x[0]], bands[y[0]]))
    for regrule in coll:
        be32(output, reg_rules[regrule])

# update country pointer now!
reg_country_ptr.set()

countrynames = countries.keys()
countrynames.sort()
for alpha2 in countrynames:
    coll = countries[alpha2]
    # struct regdb_file_reg_country
    output.write(struct.pack('>ccxxI', str(alpha2[0]), str(alpha2[1]), reg_rules_collections[coll]))

# Load RSA only now so people can use this script
# without having those libraries installed to verify
# their SQL changes
from M2Crypto import RSA

# determine signature length
key = RSA.load_key('key.priv.pem')
hash = sha.new()
hash.update(output.getvalue())
sig = key.sign(hash.digest())
# write it to file
siglen.set(len(sig))
# sign again
hash = sha.new()
hash.update(output.getvalue())
sig = key.sign(hash.digest())

output.write(sig)

outfile = open('regulatory.bin', 'w')
outfile.write(output.getvalue())
