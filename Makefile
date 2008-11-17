ifeq ($(origin $(KLIB)), undefined)
KLIB := /lib/modules/$(shell uname -r)
endif
KLIB_BUILD ?= $(KLIB)/build

ifneq ($(COMPAT_TREE),)
CFLAGS += -I$(COMPAT_TREE)/include/
endif

CRDA_LIB := "/usr/lib/crda/"

# Used locally to retrieve all pubkeys during build time
PUBKEY_DIR=pubkeys

KORG="http://www.kernel.org/pub/linux/kernel/people"

#LINVILLE_REGDB=$(KORG)/linville/regdb/latest/regulatory.bin
#LINVILLE_RSAPUB=$(KORG)/linville/regdb/linville.key.pub.pem

ATHEROS_REGDB_ASCII=$(KORG)/mcgrof/regdb/latest/db.txt
ATHEROS_REGDB_BIN=$(KORG)/mcgrof/regdb/latest/regulatory.bin

ATHEROS_RSAPUB=$(KORG)/mcgrof/regdb/atheros.key.pub.pem

# We can remove the atheros bin/ascii db once Linville has one
REGDB_ASCII=$(ATHEROS_REGDB_ASCII)
REGDB_BIN=$(ATHEROS_REGDB_BIN)

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

all: regulatory.bin crda intersect
	$(Q)$(MAKE) --no-print-directory -f Makefile verify

# Add all pubkeys here
$(PUBKEY_DIR)/$(wildcard *.pem):
	$(Q)wget -q -P $(PUBKEY_DIR)/ $(ATHEROS_RSAPUB)

regulatory.bin:
	$(Q)wget -q $(REGDB_BIN)

# Authors of regulatory.bin first need a private key, which can
# be generated with something like this:
# openssl genrsa -out your.key.priv.pem 2048
#
# You'll then need to generate the public key and publish it. You
# can generate it as follows:
# openssl rsa -in your.key.priv.pem -out your.key.pub.pem -pubout -outform PEM
#
# Then with this key you can generate regulatory.bin files like this:
# ./utils/db2bin.py regulatory.bin db.txt your.key.priv.pem
#
# You can find the source of the regulatory.bin used at $(REGDB_ASCII)

keys-%.c: utils/key2pub.py $(PUBKEY_DIR)/$(wildcard *.pem)
	$(NQ) '  GEN ' $@
	$(Q)./utils/key2pub.py --$* $(PUBKEY_DIR)/*.pem > $@

%.o: %.c regdb.h
	$(NQ) '  CC  ' $@
	$(Q)$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

crda: keys-ssl.c keys-gcrypt.c reglib.o crda.o
	$(NQ) '  LD  ' $@
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS) `pkg-config --libs libnl-1` -o $@ reglib.o crda.o


dump: keys-ssl.c keys-gcrypt.c reglib.o dump.o
	$(NQ) '  LD  ' $@
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS) -o $@ reglib.o dump.o

intersect: regulatory.bin keys-ssl.c keys-gcrypt.c reglib.o intersect.o
	$(NQ) '  LD  ' $@
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS) -o $@ reglib.o intersect.o

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
	$(NQ) '  INSTALL  udev/regulatory.rules'
	$(Q)$(MKDIR) $(DESTDIR)/etc/udev/rules.d
	$(Q)$(INSTALL) -m 644 -t $(DESTDIR)/etc/udev/rules.d/ udev/regulatory.rules

clean:
	$(Q)rm -f db.txt regulatory.bin crda dump intersect *.o *~ *.pyc keys-*.c
	$(Q)rm -rf $(PUBKEY_DIR)
	$(Q)if test -f key.priv.pem && diff -qNs test-key key.priv.pem >/dev/null ; then \
		rm -f key.priv.pem;\
	fi
