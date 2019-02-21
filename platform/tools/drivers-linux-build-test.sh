#!/bin/bash

TOP=$(readlink -f "$(dirname "$0")/../..")

"$TOP/platform/tools/drivers-linux.sh" || exit
cd "$TOP/platform/gen/drivers-linux/drivers" || exit

fail() { FAILURES="$FAILURES\n  $*" ; }

# build-test for each kernel version that has installed headers
export KSRC
for KSRC in /usr/src/linux*/ /usr/src/kernels/*/ ; do
    if ! [ -e "$KSRC/include/generated/autoconf.h" -a \
           -e "$KSRC/include/config/auto.conf" ] ; then
        continue
    fi
    make clean && make -k || fail "$KSRC"
done

# build-test the user space driver
cd ../rdma-core
./build.sh || fail "rdma-core"

if [ -v FAILURES ] ; then
echo -e "failures:$FAILURES"
exit 1
fi

echo "success"
