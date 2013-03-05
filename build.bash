#!/bin/bash

set -e

BINDIR="bin"
SRCDIR="src"

cflags=""
extras=""

if pkg-config talloc --exists
then
    CFLAGS="$CFLAGS $(pkg-config talloc --cflags)"
    CFLAGS="$CFLAGS $(pkg-config talloc --libs)"
    CFLAGS="$CFLAGS -DHAVE_TALLOC"
else
    CFLAGS="$CFLAGS -Isrc/extern/"
    CFLAGS="$CFLAGS -DNO_CONFIG_H"
    extras="$SRCDIR/extern"
fi

CFLAGS="$CFLAGS -DTEK_VERSION=\"upconfigure\""
CFLAGS="$CFLAGS -pipe -O2"

mkdir -p "$BINDIR"

ls "$SRCDIR" | grep -v "^extern$" | while read program
do
    find "$SRCDIR/$program" $extras -iname *.c | \
	xargs cc $CFLAGS -o "$BINDIR/$program"
done
