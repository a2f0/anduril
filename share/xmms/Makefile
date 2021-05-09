# Makefile for 'xmms-epic' utility.  Try to work around problems in some
# systems here.

OS=`uname -s`
CC=gcc
CFLAGS!=xmms-config --cflags
CFLAGS+=-I. 
LDFLAGS!=xmms-config --libs

all:
	$(CC) $(CFLAGS) -c epic-xmms.c
	$(CC) $(LDFLAGS) -o epic-xmms epic-xmms.o
	rm epic-xmms.o
	strip epic-xmms

clean:
	rm -f *~ *.o epic-xmms
