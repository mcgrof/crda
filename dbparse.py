#!/usr/bin/env python

import sys

band_flags = {
    'CCK': 1,
    'OFDM': 2,
}

class FreqBand(object):
    def __init__(self, start, end, bw, flags, comments=None):
        self.start = start
        self.end = end
        self.maxbw = bw
        self.flags = flags
        self.comments = comments or []

    def __cmp__(self, other):
        s = self
        o = other
        if not isinstance(o, FreqBand):
            return False
        return cmp((s.start, s.end, s.maxbw, s.flags), (o.start, o.end, o.maxbw, o.flags))

    def __hash__(self):
        s = self
        return hash((s.start, s.end, s.maxbw, s.flags))

    def __str__(self):
        flags = []
        for f, v in band_flags.iteritems():
            if self.flags & v:
                flags.append(f)
        return '<FreqBand %.3f - %.3f @ %.3f%s>' % (
                  self.start, self.end, self.maxbw, ', '.join(flags))

class PowerRestriction(object):
    def __init__(self, environment, max_ant_gain, max_ir_ptmp,
                 max_ir_ptp, max_eirp_ptmp, max_eirp_ptp,
                 comments = None):
        self.environment = environment
        self.max_ant_gain = max_ant_gain
        self.max_ir_ptmp = max_ir_ptmp
        self.max_ir_ptp = max_ir_ptp
        self.max_eirp_ptmp = max_eirp_ptmp
        self.max_eirp_ptp = max_eirp_ptp
        self.comments = comments or []

    def __cmp__(self, other):
        s = self
        o = other
        if not isinstance(o, PowerRestriction):
            return False
        return cmp((s.environment, s.max_ant_gain, s.max_ir_ptmp,
                    s.max_ir_ptp, s.max_eirp_ptmp, s.max_eirp_ptp),
                   (o.environment, o.max_ant_gain, o.max_ir_ptmp,
                    o.max_ir_ptp, o.max_eirp_ptmp, o.max_eirp_ptp))

    def __str__(self):
        return '<PowerRestriction ...>'

    def __hash__(self):
        s = self
        return hash((s.environment, s.max_ant_gain, s.max_ir_ptmp,
                     s.max_ir_ptp, s.max_eirp_ptmp, s.max_eirp_ptp))

class Country(object):
    def __init__(self, restrictions=None, comments=None):
        # tuple of (freqband, powerrestriction)
        self._restrictions = restrictions or []
        self.comments = comments or []

    def add(self, band, power):
        assert isinstance(band, FreqBand)
        assert isinstance(power, PowerRestriction)
        self._restrictions.append((band, power))
        self._restrictions.sort()

    def __contains__(self, tup):
        return tup in self._restrictions

    def __str__(self):
        r = ['(%s, %s)' % (str(b), str(p)) for b, p in self._restrictions]
        return '<Country (%s)>' % (', '.join(r))

    def _get_restrictions_tuple(self):
        return tuple(self._restrictions)
    restrictions = property(_get_restrictions_tuple)

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

    def _parse_band_def(self, bname, banddef, dupwarn=True):
        line = banddef
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
            if not f in band_flags:
                self._syntax_error("Invalid band flag")
            fl |= band_flags[f]

        b = FreqBand(start, end, bw, fl)
        self._banddup[bname] = bname
        if b in self._bandrev:
            if dupwarn:
                self._warn('Duplicate band definition ("%s" and "%s")' % (
                              bname, self._bandrev[b]))
            self._banddup[bname] = self._bandrev[b]
        self._bands[bname] = b
        self._bandrev[b] = bname
        self._bandline[bname] = self._lineno

    def _parse_band(self, line):
        try:
            bname, line = line.split(':', 1)
            if not bname:
                self._syntax_error("'band' keyword must be followed by name")
        except ValueError:
            self._syntax_error("band name must be followed by colon")

        self._parse_band_def(bname, line)

    def _parse_power(self, line):
        try:
            pname, line = line.split(':', 1)
            if not pname:
                self._syntax_error("'power' keyword must be followed by name")
        except ValueError:
            self._syntax_error("power name must be followed by colon")

        self._parse_power_def(pname, line)

    def _parse_power_def(self, pname, line, dupwarn=True):
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

        p = PowerRestriction(environ, max_ant_gain, max_ir_ptmp,
                             max_ir_ptp, max_eirp_ptmp, max_eirp_ptp)
        self._powerdup[pname] = pname
        if p in self._powerrev:
            if dupwarn:
                self._warn('Duplicate power definition ("%s" and "%s")' % (
                              pname, self._powerrev[p]))
            self._powerdup[pname] = self._powerrev[p]
        self._power[pname] = p
        self._powerrev[p] = pname
        self._powerline[pname] = self._lineno

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
            self._countries[cname] = Country()
        self._current_country = self._countries[cname]
        self._current_country_name = cname

    def _parse_country_item(self, line):
        if line[0] == '(':
            try:
                band, pname = line[1:].split('),')
                bname = 'UNNAMED %d' % self._lineno
                self._parse_band_def(bname, band, dupwarn=False)
            except:
                self._syntax_error("Badly parenthesised band definition")
        else:
            try:
                bname, pname = line.split(',', 1)
                if not bname:
                    self._syntax_error("country definition must have band")
                if not pname:
                    self._syntax_error("country definition must have power")
            except ValueError:
                self._syntax_error("country definition must have band and power")

        if pname[0] == '(':
            if not pname[-1] == ')':
                self._syntax_error("Badly parenthesised power definition")
            power = pname[1:-1]
            pname = 'UNNAMED %d' % self._lineno
            self._parse_power_def(pname, power, dupwarn=False)

        if not bname in self._bands:
            self._syntax_error("band does not exist")
        if not pname in self._power:
            self._syntax_error("power does not exist")
        self._bands_used[bname] = True
        self._power_used[pname] = True
        # de-duplicate so binary database is more compact
        bname = self._banddup[bname]
        pname = self._powerdup[pname]
        b = self._bands[bname]
        p = self._power[pname]
        if (b, p) in self._current_country:
            self._warn('Rule "%s, %s" added to "%s" twice' % (
                          bname, pname, self._current_country_name))
        else:
            self._current_country.add(b, p)

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
        self._bandline = {}
        self._powerline = {}

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
            for k in k.split(','):
                countries[k] = v
        bands = {}
        for k, v in self._bands.iteritems():
            if k in self._bands_used:
                bands[self._banddup[k]] = v
                continue
            # we de-duplicated, but don't warn again about the dupes
            if self._banddup[k] == k:
                self._lineno = self._bandline[k]
                self._warn('Unused band definition "%s"' % k)
        power = {}
        for k, v in self._power.iteritems():
            if k in self._power_used:
                power[self._powerdup[k]] = v
                continue
            # we de-duplicated, but don't warn again about the dupes
            if self._powerdup[k] == k:
                self._lineno = self._powerline[k]
                self._warn('Unused power definition "%s"' % k)
        return countries
