#!/usr/bin/env python

import sys
from M2Crypto import RSA

def print_ssl(name, val):
    sys.stdout.write('static BN_ULONG %s[] = {\n' % name)
    idx = 0
    while val[0] == '\0':
        val = val[1:]
    while len(val) % 4:
        val = '\0' + val
    vnew = []
    while len(val):
        vnew.append((val[0], val[1], val[2], val[3], ))
        val = val[4:]
    vnew.reverse()
    for v1, v2, v3, v4 in vnew:
        if not idx:
            sys.stdout.write('\t')
        sys.stdout.write('0x%.2x%.2x%.2x%.2x, ' % (ord(v1), ord(v2), ord(v3), ord(v4)))
        idx += 1
        if idx == 4:
            idx = 0
            sys.stdout.write('\n')
    if idx:
        sys.stdout.write('\n')
    sys.stdout.write('};\n\n')

def print_ssl_keys(n):
    sys.stdout.write(r'''
struct pubkey {
	struct bignum_st e, n;
};

#define KEY(data) {				\
	.d = data,				\
	.top = sizeof(data)/sizeof(data[0]),	\
}

#define KEYS(e,n)	{ KEY(e), KEY(n), }

static struct pubkey keys[] = {
''')
    for n in xrange(n + 1):
        sys.stdout.write('	KEYS(e_%d, n_%d),\n' % (n, n))
    sys.stdout.write('};\n')
    pass

def print_gcrypt(name, val):
    sys.stdout.write('static __u8 %s[] = {\n' % name)
    idx = 0
    while val[0] == '\0':
        val = val[1:]
    for v in val:
        if not idx:
            sys.stdout.write('\t')
        sys.stdout.write('0x%.2x, ' % ord(v))
        idx += 1
        if idx == 8:
            idx = 0
            sys.stdout.write('\n')
    if idx:
        sys.stdout.write('\n')
    sys.stdout.write('};\n\n')

def print_gcrypt_keys(n):
    sys.stdout.write(r'''
struct key_params {
	__u8 *e, *n;
	__u32 len_e, len_n; 
};

#define KEYS(_e, _n) {			\
	.e = _e, .len_e = sizeof(_e),	\
	.n = _n, .len_n = sizeof(_n),	\
}

static struct key_params keys[] = {
''')
    for n in xrange(n + 1):
        sys.stdout.write('	KEYS(e_%d, n_%d),\n' % (n, n))
    sys.stdout.write('};\n')
    

modes = {
    '--ssl': (print_ssl, print_ssl_keys),
    '--gcrypt': (print_gcrypt, print_gcrypt_keys),
}

try:
    mode = sys.argv[1]
    files = sys.argv[2:]
except IndexError:
    mode = None

if not mode in modes:
    print 'Usage: %s [%s] files' % (sys.argv[0], '|'.join(modes.keys()))
    sys.exit(2)

# load key
idx = 0
for f in files:
    key = RSA.load_key(f)

    modes[mode][0]('e_%d' % idx, key.e[4:])
    modes[mode][0]('n_%d' % idx, key.n[4:])

modes[mode][1](idx)
