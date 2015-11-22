TARGETS = \
	     check_seteugid \
	     connect \
	     connect2 \
	     filter \
	     import \
	     makeiprt \
	     ms2mm \
	     udpbridge

all:; @. ../lib/configure.mak; $(MAKE) -i _all
install:; @. ../lib/configure.mak; $(MAKE) -i _install

_all: $(TARGETS)

check_seteugid: check_seteugid.o $(LIBS)
	$(RMTARGET)
	$(LD) $(LDFLAGS) check_seteugid.o $(LIBS) $(SLIBS) -o $@
	sudo chown root $@
	sudo chmod 4555 $@

connect: connect.o $(LIBS)
	$(RMTARGET)
	$(LD) $(LDFLAGS) connect.o $(LIBS) $(SLIBS) -o $@

connect2: connect2.o $(LIBS)
	$(RMTARGET)
	$(LD) $(LDFLAGS) connect2.o $(LIBS) $(SLIBS) -o $@

filter: filter.o $(LIBS)
	$(RMTARGET)
	$(LD) $(LDFLAGS) filter.o $(LIBS) $(SLIBS) -o $@

import: import.o $(LIBS)
	$(RMTARGET)
	$(LD) $(LDFLAGS) import.o $(LIBS) $(SLIBS) -o $@

makeiprt: makeiprt.o $(LIBS)
	$(RMTARGET)
	$(LD) $(LDFLAGS) makeiprt.o $(LIBS) $(SLIBS) -o $@

ms2mm: ms2mm.o $(LIBS)
	$(RMTARGET)
	$(LD) $(LDFLAGS) ms2mm.o $(LIBS) $(SLIBS) -o $@

udpbridge: udpbridge.o $(LIBS)
	$(RMTARGET)
	$(LD) $(LDFLAGS) udpbridge.o $(LIBS) $(SLIBS) -o $@

_install: \
	/usr/local/bin/import

/usr/local/bin/import: import
	$(MKDIR)
	$(RMTARGET)
	cp $? $@

clean:; @rm -f *.o $(TARGETS) *~ \#*\#
###
check_seteugid.o:\
	../lib/seteugid.h\
	check_seteugid.c
connect.o:\
	../lib/configure.h\
	connect.c
connect2.o:\
	connect2.c
filter.o:\
	filter.c
import.o:\
	import.c
makeiprt.o:\
	../lib/configure.h\
	makeiprt.c
ms2mm.o:\
	ms2mm.c
pty-test.o:\
	pty-test.c
udpbridge.o:\
	udpbridge.c
