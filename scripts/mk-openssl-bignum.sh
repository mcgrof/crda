#!/bin/sh

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
	
	echo 'static BN_ULONG e_1[] = {' $exponent', };'
	echo ''
	echo -n 'static BN_ULONG n_1[] = {'
	modulus=$(echo $modulus | sed 's/^00//')
	echo $modulus | sed 's/\(........\)/\t0x\1,\n/g' | tac
	echo '};'
)
