#!/bin/bash

set -e

DBFILE=regulatory.bin

flen=$(stat -c '%s' regulatory.bin)
tmp=$(mktemp)
tmpdata=$(mktemp)
dd if="$DBFILE" of="$tmp" bs=1 count=128 skip=$((flen - 128)) 2>/dev/null
dd if="$DBFILE" of="$tmpdata" bs=1 count=$((flen - 128)) 2>/dev/null
openssl dgst -sha1 -verify test-key.pub.pem -signature "$tmp" "$tmpdata"
rm -f "$tmp"
rm -f "$tmpdata"
