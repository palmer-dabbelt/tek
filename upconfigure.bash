#!/bin/bash

BINDIR="bin"
SRCDIR="src"

CFLAGS=""
CFLAGS="$CFLAGS $(pkg-config talloc --cflags)"
CFLAGS="$CFLAGS $(pkg-config talloc --libs)"
CFLAGS="$CFLAGS -DTEK_VERSION=\"upconfigure\""

mkdir -p "$BINDIR"

ls "$SRCDIR" | while read program
do
    find "$SRCDIR/$program" -iname *.c | \
	xargs cc $CFLAGS -o "$BINDIR/$program"
done
