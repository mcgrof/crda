#!/usr/bin/env python

import sys

power_flags = {
    'OFDM': 2,
    'CCK': 1,
}

class SyntaxError(Exception):
    pass

class DBParser(object):
    def __init__(self):
        pass

    def _syntax_error(self, txt=None):
        txt = txt and ' (%s)' % txt or ''
        raise SyntaxError("Syntax error in line %d%s" % (self._lineno, txt))

    def _warn(self, txt):
        sys.stderr.write("Warning (line %d): %s\n" % (self._lineno, txt))

    def _parse_band(self, line):
        try:
            bname, line = line.split(':', 1)
            if not bname:
                self._syntax_error("'band' keyword must be followed by name")
        except ValueError:
            self._syntax_error("band name must be followed by colon")

        try:
            freqs, line = line.split(',', 1)
        except ValueError:
            freqs = line
            line = ''

        flags = [f.upper() for f in line.split(',') if f]

        try:
            freqs, bw = freqs.split('@')
            bw = float(bw)
        except ValueError:
            bw = 20.0

        try:
            start, end = freqs.split('-')
            start = float(start)
            end = float(end)
        except ValueError:
            self._syntax_error("band must have frequency range")

        fl = 0
        for f in flags:
            if not f in power_flags:
                self._syntax_error("Invalid band flag")
            fl |= power_flags[f]

        b = (start, end, bw, fl)
        self._banddup[bname] = bname
        if b in self._bandrev:
            self._warn('Duplicate band definition ("%s" and "%s")' % (
                          bname, self._bandrev[b]))
            self._banddup[bname] = self._bandrev[b]
        self._bands[bname] = b
        self._bandrev[b] = bname

    def _parse_power(self, line):
        try:
            pname, line = line.split(':', 1)
            if not pname:
                self._syntax_error("'power' keyword must be followed by name")
        except ValueError:
            self._syntax_error("power name must be followed by colon")

        try:
            (environ,
             max_ant_gain,
             max_ir_ptmp,
             max_ir_ptp,
             max_eirp_ptmp,
             max_eirp_ptp) = line.split(',')
            max_ant_gain = float(max_ant_gain)
            max_ir_ptmp = float(max_ir_ptmp)
            max_ir_ptp = float(max_ir_ptp)
            max_eirp_ptmp = float(max_eirp_ptmp)
            max_eirp_ptp = float(max_eirp_ptp)
        except ValueError:
            self._syntax_error("invalid power data")

        if environ == 'OI':
            environ = ' '
        if not environ in ('I', 'O', ' '):
            self._syntax_error("Invalid environment specifier")

        p = (environ, max_ant_gain, max_ir_ptmp,
             max_ir_ptp, max_eirp_ptmp, max_eirp_ptp)
        self._powerdup[pname] = pname
        if p in self._powerrev:
            self._warn('Duplicate power definition ("%s" and "%s")' % (
                          pname, self._powerrev[p]))
            self._powerdup[pname] = self._powerrev[p]
        self._power[pname] = p
        self._powerrev[p] = pname

    def _parse_country(self, line):
        try:
            cname, line = line.split(':', 1)
            if not cname:
                self._syntax_error("'country' keyword must be followed by name")
            if line:
                self._syntax_error("extra data at end of country line")
        except ValueError:
            self._syntax_error("country name must be followed by colon")

        if not cname in self._countries:
            self._countries[cname] = []
        self._current_country = self._countries[cname]
        self._current_country_name = cname

    def _parse_country_item(self, line):
        try:
            bname, pname = line.split(',', 1)
            if not bname:
                self._syntax_error("country definition must have band name")
            if not pname:
                self._syntax_error("country definition must have power name")
        except ValueError:
            self._syntax_error("country definition must have band and power names")
        if not bname in self._bands:
            self._syntax_error("band does not exist")
        if not pname in self._power:
            self._syntax_error("power does not exist")
        bname = self._banddup[bname]
        pname = self._powerdup[pname]
        # de-duplicate so binary database is more compact
        self._bands_used[bname] = True
        self._power_used[pname] = True
        tup = (bname, pname)
        if tup in self._current_country:
            self._warn('Rule "%s, %s" added to "%s" twice' % (
                          bname, pname, self._current_country_name))
        else:
            self._current_country.append((bname, pname))

    def parse(self, f):
        self._current_country = None
        self._bands = {}
        self._power = {}
        self._countries = {}
        self._bands_used = {}
        self._power_used = {}
        self._bandrev = {}
        self._powerrev = {}
        self._banddup = {}
        self._powerdup = {}

        self._lineno = 0
        for line in f:
            self._lineno += 1
            line = line.strip()
            line = line.replace(' ', '').replace('\t', '')
            line = line.split('#')[0]
            if not line:
                continue
            if line[0:4] == 'band':
                self._parse_band(line[4:])
                self._current_country = None
            elif line[0:5] == 'power':
                self._parse_power(line[5:])
                self._current_country = None
            elif line[0:7] == 'country':
                self._parse_country(line[7:])
            elif self._current_country is not None:
                self._parse_country_item(line)
            else:
                self._syntax_error("Expected band, power or country definition")

        countries = {}
        for k, v in self._countries.iteritems():
            v.sort()
            for k in k.split(','):
                countries[k] = tuple(v)
        bands = {}
        for k, v in self._bands.iteritems():
            if k in self._bands_used:
                bands[k] = v
                continue
            # we de-duplicated, but don't warn again about the dupes
            if self._banddup[k] == k:
                self._warn('Unused band definition "%s"' % k)
        power = {}
        for k, v in self._power.iteritems():
            if k in self._power_used:
                power[k] = v
                continue
            # we de-duplicated, but don't warn again about the dupes
            if self._powerdup[k] == k:
                self._warn('Unused power definition "%s"' % k)
        return bands, power, countries

def create_rules(countries):
    result = {}
    for c in countries.itervalues():
        for rule in c:
            result[rule] = 1
    return result.keys()

def create_collections(countries):
    result = {}
    for c in countries.itervalues():
        c = tuple(c)
        result[c] = 1
    return result.keys()

if __name__ == '__main__':
    import sys
    p = DBParser()
    b, p, c = p.parse(sys.stdin)
    print b
    print p
    print c
    print create_rules(c)
    print create_collections(c)
