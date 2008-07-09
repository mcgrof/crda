CFLAGS += -Wall -g3
#CFLAGS += -DUSE_OPENSSL
#LDFLAGS += -lssl
ifneq ($(COMPAT_TREE),)
CFLAGS += -I$(COMPAT_TREE)/include/
endif
CFLAGS += -DUSE_GCRYPT
LDFLAGS += -lgcrypt

CRDA_LIB="/usr/lib//crda/"

all:	regulatory.bin warn crda
	@$(MAKE) --no-print-directory -f Makefile verify

regulatory.bin:	db2bin.py key.priv.pem db.txt dbparse.py
	@./db2bin.py regulatory.bin db.txt key.priv.pem

crda: keys-gcrypt.c crda.c regdb.h
	$(CC) $(CFLAGS) $(LDFLAGS) -lnl -o $@ crda.c

clean:
	@rm -f regulatory.bin dump *~ *.pyc keys-*.c

warn:
	@if test !  -f key.priv.pem || diff -qNs test-key key.priv.pem >/dev/null ; then \
	echo '**************************************';\
	echo '**  WARNING!                        **';\
	echo '**  No key found, using TEST key!   **';\
	echo '**************************************';\
	fi

key.priv.pem:
	cp test-key key.priv.pem

generate_key:
	openssl genrsa -out key.priv.pem 2048

dump:	dump.c regdb.h keys-ssl.c keys-gcrypt.c
	$(CC) $(CFLAGS) $(LDFLAGS) dump.c -o dump

keys-ssl.c: key2pub.py $(wildcard *.pem)
	@./key2pub.py --ssl *.pem > keys-ssl.c

keys-gcrypt.c: key2pub.py *.pem
	@./key2pub.py --gcrypt *.pem > keys-gcrypt.c

verify: dump
	@./dump regulatory.bin >/dev/null

install: regulatory.bin crda
	mkdir -p $(CRDA_LIB)
	install regulatory.bin $(CRDA_LIB)
	install crda /sbin/
