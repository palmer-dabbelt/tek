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
    echo "This tool requires the 'talloc' library"
    echo "   For Debian-based distros (e.g. Ubuntu):"
    echo "      apt-get install libtalloc-dev"
    echo "   For RPM-based distros (e.g. Fedora):"
    echo "      yum install libtalloc-devel"
    echo "   For OS X:"
    echo "      port install talloc"
    exit 1
fi

CFLAGS="$CFLAGS -DTEK_VERSION=\"upconfigure\""
CFLAGS="$CFLAGS -pipe -O2"

mkdir -p "$BINDIR"

ls "$SRCDIR" | while read program
do
    find "$SRCDIR/$program" $extras -iname *.c | \
	xargs cc $CFLAGS -o "$BINDIR/$program"
done
