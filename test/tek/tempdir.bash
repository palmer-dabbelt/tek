set -ex

tempdir=`mktemp -d -t ptest-tek.XXXXXXXXXX`
trap "rm -rf $tempdir" EXIT
cd $tempdir
