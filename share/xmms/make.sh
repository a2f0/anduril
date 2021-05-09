#!/bin/sh

CFL=`xmms-config --cflags`
LFL=`xmms-config --libs`
echo gcc $CFL -c epic-xmms.c
gcc $CFL -c epic-xmms.c
echo gcc $LFL -o epic-xmms epic-xmms.o
gcc $LFL -o epic-xmms epic-xmms.o
echo rm epic-xmms.o
rm epic-xmms.o
echo strip epic-xmms
strip epic-xmms

