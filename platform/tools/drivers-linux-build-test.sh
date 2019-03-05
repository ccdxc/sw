#!/bin/bash

TOP=$(readlink -f "$(dirname "$0")/../..")
MARK='################################'

echo -e "\n$MARK\ndrivers package\n$MARK\n"
"$TOP/platform/tools/drivers-linux.sh" || exit
cd "$TOP/platform/gen/drivers-linux/drivers" || exit

fail() { FAILURES="$FAILURES\n  $*" ; }

export KSRC=
export OFA_KSRC=

# build-test for each kernel version that has installed headers
for KSRC in /usr/src/linux*/ /usr/src/kernels/*/ ; do
    if ! [ -e "$KSRC/include/generated/autoconf.h" -a \
           -e "$KSRC/include/config/auto.conf" ] ; then
        continue
    fi
    echo -e "\n$MARK\n$KSRC\n$MARK\n"
    make clean && make -k || fail "$KSRC"
done

# build-test for ofed rdma stack
for OFA_KSRC in /usr/src/ofa_kernel/* ; do
    KSRC="/usr/src/linux-headers-$(basename "$OFA_KSRC")"
    if ! [ -e "$KSRC/include/generated/autoconf.h" -a \
           -e "$KSRC/include/config/auto.conf" ] ; then
        continue
    fi
    echo -e "\n$MARK\n$OFA_KSRC\n$MARK\n"
    make clean && make -k || fail "$OFA_KSRC"
done

# build-test the user space driver
echo -e "\n$MARK\nrdma-core\n$MARK\n"
cd ../rdma-core
./build.sh || fail "rdma-core"

echo -e "\n$MARK\nsummary\n$MARK\n"

if [ -v FAILURES ] ; then
echo -e "failures:$FAILURES"
exit 1
fi

echo "success"
