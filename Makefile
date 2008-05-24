CFLAGS += -Wall -g3
#CFLAGS += -DVERIFY_SIGNATURE -DUSE_OPENSSL
#LDFLAGS += -lssl
CFLAGS += -DVERIFY_SIGNATURE -DUSE_GCRYPT
LDFLAGS += -lgcrypt

all:	regulatory.bin verify

regulatory.bin:	db2bin.py key.priv.pem db.txt dbparse.py
	@./db2bin.py

clean:
	@rm -f regulatory.bin dump *~ *.pyc keys-*.c

generate_keys:
	openssl genrsa -out key.priv.pem 2048

dump:	dump.c regdb.h keys-ssl.c keys-gcrypt.c
	$(CC) $(CFLAGS) $(LDFLAGS) dump.c -o dump

keys-ssl.c: key2pub.py *.priv.pem
	@./key2pub.py --ssl *.priv.pem > keys-ssl.c

keys-gcrypt.c: key2pub.py *.priv.pem
	@./key2pub.py --gcrypt *.priv.pem > keys-gcrypt.c

verify: dump
	@./dump regulatory.bin >/dev/null
