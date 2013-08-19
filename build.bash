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
    if [[ "$1" != "--allow-internal-talloc" ]]
    then
        cat <<EOF
WARNING: Using internal talloc library

This tool requires the 'talloc' library
   For Debian-based distros (e.g. Ubuntu):
      apt-get install libtalloc-dev
   For RPM-based distros (e.g. Fedora):
      yum install libtalloc-devel
   For OS X:
      port install talloc

Re-call this script with --allow-internal-talloc to enable an internal
talloc library to be used.  This is NOT recommended.
EOF

        CFLAGS="$CFLAGS -Isrc/extern/"
        CFLAGS="$CFLAGS -DNO_CONFIG_H"
        extras="$SRCDIR/extern/extern/talloc.c"
    fi
fi

CFLAGS="$CFLAGS -DTEK_VERSION=\"upconfigure\""
CFLAGS="$CFLAGS -pipe -O2"

mkdir -p "$BINDIR"

ls "$SRCDIR" | grep -v "^extern$" | while read program
do
    find "$SRCDIR/$program" $extras -iname *.c | \
	xargs cc $CFLAGS -o "$BINDIR/$program"
done
