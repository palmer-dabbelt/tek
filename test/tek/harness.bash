export PATH="$(dirname "$PTEST_BINARY"):$PATH"

$PTEST_BINARY
yes Q | make -j1
