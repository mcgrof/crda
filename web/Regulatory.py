# -*- coding: iso-8859-1 -*-
"""
    Regulatory Database

    @copyright: 2008 Johannes Berg
    @license: GNU GPL, see COPYING for details.
"""

import codecs
from dbparse import DBParser, band_flags

Dependencies = ["time"]

def _country(macro, countries, code):
    result = []

    f = macro.formatter

    result.extend([
        f.heading(1, 1),
        f.text('Regulatory definition for %s' % _get_iso_code(code)),
        f.heading(0, 1),
    ])

    try:
        country = countries[code]
    except:
        result.append(f.text('No information available'))
        return ''.join(result)
    

    if country.comments:
        result.extend([
            f.preformatted(1),
            f.text('\n'.join(country.comments)),
            f.preformatted(0),
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

    for b, p in country.restrictions:
        flags = []
        for flag, val in band_flags.iteritems():
            if b.flags & val:
                flags.append(flag)
        e = {
            'O': 'Outdoor',
            'I': 'Indoor',
            ' ': 'Out- & Indoor'
        }[p.environment]
        result.extend([
            f.table_row(1),
              f.table_cell(1),
                f.text('%.3f - %.3f' % (b.start, b.end)),
              f.table_cell(0),
              f.table_cell(1),
                f.text('%.3f' % (b.maxbw,)),
              f.table_cell(0),
              f.table_cell(1),
                f.text(', '.join(flags)),
              f.table_cell(0),
              f.table_cell(1),
                f.text(e),
              f.table_cell(0),
              f.table_cell(1),
                f.text('%.3f' % p.max_ant_gain),
              f.table_cell(0),
              f.table_cell(1),
                f.text('%.3f' % p.max_ir_ptmp),
              f.table_cell(0),
              f.table_cell(1),
                f.text('%.3f' % p.max_ir_ptp),
              f.table_cell(0),
              f.table_cell(1),
                f.text('%.3f' % p.max_eirp_ptmp),
              f.table_cell(0),
              f.table_cell(1),
                f.text('%.3f' % p.max_eirp_ptp),
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
            c, name = line.split('\t')
            _iso_list[c] = name
    return _iso_list.get(code, 'Unknown (%s)' % code)

def macro_Regulatory(macro):
    _ = macro.request.getText
    request = macro.request
    f = macro.formatter

    country = request.form.get('alpha2', [None])[0]

    dbpath = '/tmp/db.txt'
    if hasattr(request.cfg, 'regdb_path'):
        dbpath = request.cfg.regdb_path
    countries = DBParser().parse(open(dbpath))

    if country:
        return _country(macro, countries, country)

    countries = countries.keys()
    countries = [(_get_iso_code(code), code) for code in countries]
    countries.sort()

    result = []

    result.extend([
        f.heading(1, 1),
        f.text('Countries'),
        f.heading(0, 1),
    ])

    result.append(f.bullet_list(1))
    for name, code in countries:
        result.extend([
          f.listitem(1),
          request.page.link_to(request, name, querystr={'alpha2': code}),
          f.listitem(0),
        ])
    result.append(f.bullet_list(0))

    return ''.join(result)

