set -ex

tempdir=`mktemp -d -t ptest-texpp.XXXXXXXXXX`
trap "rm -rf $tempdir" EXIT
cd $tempdir
