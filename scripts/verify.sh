#!/bin/bash

set -e

echo -n 'Verifying database signature: '

if ! which openssl >/dev/null ; then
	echo "OpenSSL not installed, cannot verify"
	exit 0
fi

DBFILE=regulatory.bin

flen=$(stat -c '%s' regulatory.bin)
siglen=$(dd if=regulatory.bin bs=1 count=4 skip=16 2>/dev/null |hexdump -e '"0x%08x"')
siglen=$((siglen))
tmp=$(mktemp)
tmpdata=$(mktemp)
dd if="$DBFILE" of="$tmp" bs=1 count=$siglen skip=$((flen - siglen)) 2>/dev/null
dd if="$DBFILE" of="$tmpdata" bs=1 count=$((flen - siglen)) 2>/dev/null
openssl dgst -sha1 -verify key.pub.pem -signature "$tmp" "$tmpdata"
exit=$?
rm -f "$tmp" "$tmpdata"
