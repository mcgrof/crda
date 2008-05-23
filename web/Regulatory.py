# -*- coding: iso-8859-1 -*-
"""
    Regulatory Database

    @copyright: 2008 Johannes Berg
    @license: GNU GPL, see COPYING for details.
"""

import codecs
from dbparse import DBParser, band_flags

Dependencies = ["time"]

def _country(macro, bpc, code):
    band, power, country = bpc
    result = []

    f = macro.formatter

    result.extend([
        f.heading(1, 1),
        f.text('Regulatory definition for %s' % _get_iso_code(code)),
        f.heading(0, 1),
    ])

    result.append(f.table(1))
    result.extend([
        f.table_row(1),
          f.table_cell(1), f.strong(1),
            f.text('Band (MHz)'),
          f.strong(0), f.table_cell(0),
          f.table_cell(1), f.strong(1),
            f.text('Max BW (MHz)'),
          f.strong(0), f.table_cell(0),
          f.table_cell(1), f.strong(1),
            f.text('Flags'),
          f.strong(0), f.table_cell(0),
          f.table_cell(1), f.strong(1),
            f.text('Environment'),
          f.strong(0), f.table_cell(0),
          f.table_cell(1), f.strong(1),
            f.text('Max antenna gain (dBi)'),
          f.strong(0), f.table_cell(0),
          f.table_cell(1), f.strong(1),
            f.text('Max IR PTMP (dBm)'),
          f.strong(0), f.table_cell(0),
          f.table_cell(1), f.strong(1),
            f.text('Max IR PTP (dBm)'),
          f.strong(0), f.table_cell(0),
          f.table_cell(1), f.strong(1),
            f.text('Max EIRP PTMP (dBm)'),
          f.strong(0), f.table_cell(0),
          f.table_cell(1), f.strong(1),
            f.text('Max EIRP PTP (dBm)'),
          f.strong(0), f.table_cell(0),
        f.table_row(0),
    ])

    l = list(country[code])
    l.sort(cmp=lambda x,y: cmp(band[x[0]], band[y[0]]))
    for b, p in l:
        b = band[b]
        p = power[p]
        flags = []
        for flag, val in band_flags.iteritems():
            if b[3] & val:
                flags.append(flag)
        e = {
            'O': 'Outdoor',
            'I': 'Indoor',
            ' ': 'Out- & Indoor'
        }[p[0]]
        result.extend([
            f.table_row(1),
              f.table_cell(1),
                f.text('%.3f - %.3f' % (b[0], b[1])),
              f.table_cell(0),
              f.table_cell(1),
                f.text('%.3f' % (b[2],)),
              f.table_cell(0),
              f.table_cell(1),
                f.text(', '.join(flags)),
              f.table_cell(0),
              f.table_cell(1),
                f.text(e),
              f.table_cell(0),
              f.table_cell(1),
                f.text('%.3f' % p[1]),
              f.table_cell(0),
              f.table_cell(1),
                f.text('%.3f' % p[2]),
              f.table_cell(0),
              f.table_cell(1),
                f.text('%.3f' % p[3]),
              f.table_cell(0),
              f.table_cell(1),
                f.text('%.3f' % p[4]),
              f.table_cell(0),
              f.table_cell(1),
                f.text('%.3f' % p[5]),
              f.table_cell(0),
            f.table_row(0),
        ])
    
    result.append(f.table(0))

    result.append(f.linebreak(0))
    result.append(f.linebreak(0))
    result.append(macro.request.page.link_to(macro.request, 'return to country list'))
    return ''.join(result)

_iso_list = {}

def _get_iso_code(code):
    if not _iso_list:
        for line in codecs.open('/usr/share/iso-codes/iso_3166.tab', encoding='utf-8'):
            line = line.strip()
            code, name = line.split('\t')
            _iso_list[code] = name
        _iso_list['00'] = 'Debug 1'
        _iso_list['01'] = 'Debug 2'
        _iso_list['02'] = 'Debug 3'
        _iso_list['03'] = 'Debug 4'
    return _iso_list.get(code, 'Unknown (%s)' % code)

def macro_Regulatory(macro):
    _ = macro.request.getText
    request = macro.request

    country = request.form.get('alpha2', [None])[0]

    dbpath = '/tmp/db.txt'
    if hasattr(request.cfg, 'regdb_path'):
        dbpath = request.cfg.regdb_path
    bpc = DBParser().parse(open(dbpath))

    if country:
        return _country(macro, bpc, country)

    band, power, country = bpc
    countries = country.keys()
    countries = [(_get_iso_code(code), code) for code in countries]
    countries.sort()

    result = []

    result.extend([
        f.heading(1, 1),
        f.text('Countries'),
        f.heading(0, 1),
    ])

    result.append(macro.formatter.bullet_list(1))
    for name, code in countries:
        result.extend([
          macro.formatter.listitem(1),
          request.page.link_to(request, name, querystr={'alpha2': code}),
          macro.formatter.listitem(0),
        ])
    result.append(macro.formatter.bullet_list(0))

    return ''.join(result)

