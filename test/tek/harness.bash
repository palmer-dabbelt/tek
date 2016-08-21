export PATH="$(dirname "$PTEST_BINARY"):$PATH"

$PTEST_BINARY $TEKARGS

cat Makefile

echo "" | make -j1
