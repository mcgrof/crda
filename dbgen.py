#!/usr/bin/env python

from pysqlite2 import dbapi2 as db
from cStringIO import StringIO
import struct
from M2Crypto import RSA
import sha

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

regdb = db.connect('regulatory.sqlite')
cursor = regdb.cursor()

output = StringIO()

# struct regdb_file_header
be32(output, MAGIC)
be32(output, VERSION)
reg_country_ptr = PTR(output)
# add number of countries
cursor.execute('SELECT COUNT(*) FROM reg_country;')
be32(output, cursor.fetchone()[0])

power_rules = {}
# just to make sure db scheme isn't bad
cursor.execute('''SELECT power_rule_id,
                         environment_cap,
                         max_antenna_gain_mbi,
                         max_ir_ptmp_mbm,
                         max_ir_ptp_mbm,
                         max_eirp_pmtp_mbm,
                         max_eirp_ptp_mbm FROM power_rule;''')
for pr in cursor:
    power_rule_id = pr[0]
    power_rules[power_rule_id] = output.tell()
    # struct regdb_file_power_rule
    output.write(struct.pack('>cxxxIIIII', str(pr[1]), *pr[2:]))

freq_ranges = {}
# just to make sure db scheme isn't bad
cursor.execute('''SELECT freq_range_id,
                         start_freq_khz,
                         end_freq_khz,
                         max_bandwidth_khz,
                         modulation_cap,
                         misc_restrictions FROM freq_range;''')
for fr in cursor:
    freq_range_id = fr[0]
    freq_ranges[freq_range_id] = output.tell()
    # struct regdb_file_freq_range
    output.write(struct.pack('>IIIII', *fr[1:]))


reg_rules = {}
# just to make sure db scheme isn't bad
cursor.execute('SELECT reg_rule_id, freq_range_id, power_rule_id FROM reg_rule;')
for reg_rule in cursor:
    reg_rule_id, freq_range_id, power_rule_id = reg_rule
    reg_rules[reg_rule_id] = output.tell()
    # struct regdb_file_reg_rule
    output.write(struct.pack('>II', freq_ranges[freq_range_id], power_rules[power_rule_id]))


collection_data = {}
# just to make sure db scheme isn't bad
cursor.execute('SELECT entry_id, reg_collection_id, reg_rule_id FROM reg_rules_collection;')
for coll in cursor:
    entry_id, reg_collection_id, reg_rule_id = coll
    l = collection_data.get(reg_collection_id, [])
    l.append(reg_rule_id)
    collection_data[reg_collection_id] = l

reg_rules_collections = {}

for rc, l in collection_data.iteritems():
    reg_rules_collections[rc] = output.tell()
    # struct regdb_file_reg_rules_collection
    be32(output, len(l))
    for regrule in l:
        be32(output, reg_rules[regrule])

# update country pointer now!
reg_country_ptr.set()

# just to make sure db scheme isn't bad
cursor.execute('SELECT reg_country_id, alpha2, reg_collection_id FROM reg_country;')
for country in cursor:
    reg_country_id, alpha2, reg_collection_id = country
    # struct regdb_file_reg_country
    output.write(struct.pack('>ccxxI', str(alpha2[0]), str(alpha2[1]), reg_rules_collections[reg_collection_id]))

key = RSA.load_key('test-key.priv.pem')
hash = sha.new()
hash.update(output.getvalue())
sig = key.sign(hash.digest())
assert len(sig) == 128
output.write(sig)

outfile = open('regulatory.bin', 'w')
outfile.write(output.getvalue())
