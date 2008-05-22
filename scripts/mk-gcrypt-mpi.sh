#!/bin/bash

openssl rsa -text -in key.priv.pem 2>/dev/null |\
	sed 's/^Private//;T;d' |\
	tr '\n' '\t' |\
	sed 's/privateExponent:.*//' |\
	sed 's/publicExponent:/\npublicExponent:/' |\
	sed 's/\s*//g' |\
	sed 's/publicExponent:\([^(]*\)(.*/\1/' |\
	sed 's/^modulus://' |\
	sed 's/://g' |\
(
	read modulus
	read exponent
	
	echo 'static __u8 e_1[] = {' $((exponent>>24 & 0xFF)), $((exponent>>16 & 0xFF)), $((exponent>>8 & 0xFF)), $((exponent & 0xFF)), '};'
	echo ''
	echo -n 'static __u8 n_1[] = {'
	modulus=$(echo $modulus | sed 's/^00//')
	echo $modulus | sed 's/\(..\)\(..\)\(..\)\(..\)/0x\1,0x\2,0x\3,0x\4,\n/g'
	echo '};'
)
