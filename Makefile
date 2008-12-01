# Modify as you see fit, note this is built into crda,
# so if you change it here you will have to change crda.c
REG_BIN?=/usr/lib/crda/regulatory.bin

# Used locally to retrieve all pubkeys during build time
PUBKEY_DIR=pubkeys

CFLAGS += -Wall -g
#CFLAGS += -DUSE_OPENSSL `pkg-config --cflags openssl`
#LDLIBS += `pkg-config --libs openssl`
CFLAGS += -DUSE_GCRYPT
LDLIBS += -lgcrypt

MKDIR ?= mkdir -p
INSTALL ?= install

ifeq ($(V),1)
Q=
NQ=@true
else
Q=@
NQ=@echo
endif

all: crda intersect
	$(Q)$(MAKE) --no-print-directory -f Makefile verify

keys-%.c: utils/key2pub.py $(PUBKEY_DIR)/$(wildcard *.pem)
	$(NQ) '  GEN ' $@
	$(Q)./utils/key2pub.py --$* $(PUBKEY_DIR)/*.pem > $@

%.o: %.c regdb.h
	$(NQ) '  CC  ' $@
	$(Q)$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

crda: keys-ssl.c keys-gcrypt.c reglib.o crda.o
	$(NQ) '  LD  ' $@
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS) -o $@ reglib.o crda.o `pkg-config --libs libnl-1` $(LDLIBS)

regdbdump: keys-ssl.c keys-gcrypt.c reglib.o regdbdump.o
	$(NQ) '  LD  ' $@
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS) -o $@ reglib.o regdbdump.o $(LDLIBS)

intersect: keys-ssl.c keys-gcrypt.c reglib.o intersect.o
	$(NQ) '  LD  ' $@
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS) -o $@ reglib.o intersect.o $(LDLIBS)

verify: $(REG_BIN) regdbdump
	$(NQ) '  CHK  $(REG_BIN)'
	$(Q)./regdbdump $(REG_BIN) >/dev/null

install: crda
	$(NQ) '  INSTALL  crda'
	$(Q)$(MKDIR) $(DESTDIR)/sbin
	$(Q)$(INSTALL) -m 755 -t $(DESTDIR)/sbin/ crda
	$(NQ) '  INSTALL  regdbdump'
	$(Q)$(INSTALL) -m 755 -t $(DESTDIR)/sbin/ regdbdump
	$(NQ) '  INSTALL  regulatory.rules'
	$(Q)$(MKDIR) $(DESTDIR)/etc/udev/rules.d
	$(Q)$(INSTALL) -m 644 -t $(DESTDIR)/etc/udev/rules.d/ udev/regulatory.rules

clean:
	$(Q)rm -f crda regdbdump intersect *.o *~ *.pyc keys-*.c
