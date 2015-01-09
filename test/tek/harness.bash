export PATH="$(dirname "$PTEST_BINARY"):$PATH"

$PTEST_BINARY

cat Makefile

yes Q | make -j1
