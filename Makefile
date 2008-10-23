ifeq ($(origin $(KLIB)), undefined)
KLIB := /lib/modules/$(shell uname -r)
endif
KLIB_BUILD ?= $(KLIB)/build

ifneq ($(COMPAT_TREE),)
CFLAGS += -I$(COMPAT_TREE)/include/
endif

CRDA_LIB := "/usr/lib/crda/"

CFLAGS += -Wall -g -I$(KLIB_BUILD)/include
#CFLAGS += -DUSE_OPENSSL `pkg-config --cflags openssl`
#LDFLAGS += `pkg-config --libs openssl`
CFLAGS += -DUSE_GCRYPT
LDFLAGS += -lgcrypt

MKDIR ?= mkdir -p
INSTALL ?= install

ifeq ($(V),1)
Q=
NQ=@true
else
Q=@
NQ=@echo
endif

all: regulatory.bin warn crda
	$(Q)$(MAKE) --no-print-directory -f Makefile verify

regulatory.bin: dbparse.py db2bin.py key.priv.pem db.txt
	$(NQ) '  GEN ' $@
	$(Q)./db2bin.py $@ db.txt key.priv.pem

keys-%.c: key2pub.py $(wildcard *.pem)
	$(NQ) '  GEN ' $@
	$(Q)./key2pub.py --$* *.pem > $@

%.o: %.c regdb.h
	$(NQ) '  CC  ' $@
	$(Q)$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

crda: keys-ssl.c keys-gcrypt.c regdb.o crda.o
	$(NQ) '  LD  ' $@
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS) `pkg-config --libs libnl-1` -o $@ regdb.o crda.o

warn:
	@if test ! -f key.priv.pem || diff -qNs test-key key.priv.pem >/dev/null ; then \
	echo '**************************************';\
	echo '**  WARNING!                        **';\
	echo '**  No key found, using TEST key!   **';\
	echo '**************************************';\
	fi

key.priv.pem:
	$(Q)cp test-key key.priv.pem

generate_key:
	$(Q)openssl genrsa -out key.priv.pem 2048

dump: keys-ssl.c keys-gcrypt.c regdb.o dump.o
	$(NQ) '  LD  ' $@
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS) -o $@ regdb.o dump.o

verify: regulatory.bin dump
	$(NQ) '  CHK  regulatory.bin'
	$(Q)./dump regulatory.bin >/dev/null

install: regulatory.bin crda
	$(NQ) '  INSTALL  crda'
	$(Q)$(MKDIR) $(DESTDIR)/sbin
	$(Q)$(INSTALL) -m 755 -t $(DESTDIR)/sbin/ crda
	$(NQ) '  INSTALL  regulatory.bin'
	$(Q)$(MKDIR) $(DESTDIR)$(CRDA_LIB)
	$(Q)$(INSTALL) -m 644 -t $(DESTDIR)$(CRDA_LIB) regulatory.bin
	$(NQ) '  INSTALL  regulatory.rules'
	$(Q)$(MKDIR) $(DESTDIR)/etc/udev/rules.d
	$(Q)$(INSTALL) -m 644 -t $(DESTDIR)/etc/udev/rules.d/ regulatory.rules

clean:
	$(Q)rm -f regulatory.bin crda dump *.o *~ *.pyc keys-*.c
	$(Q)if test -f key.priv.pem && diff -qNs test-key key.priv.pem >/dev/null ; then \
		rm -f key.priv.pem;\
	fi
