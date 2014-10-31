set -ex

tempdir=`mktemp -d -t ptest-texstrip.XXXXXXXXXX`
trap "rm -rf $tempdir" EXIT
cd $tempdir
