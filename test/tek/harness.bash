export PATH="$(dirname "$PTEST_BINARY"):$PATH"

$PTEST_BINARY

cat Makefile

echo "" | make -j1
