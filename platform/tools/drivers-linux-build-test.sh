#!/bin/bash

# Build test linux drivers with various kernel versions.

# Docker details
#
# Running the docker
#
# - The latest version is listed in: platform/drivers/linux/box.rb
# - Launch the docker
#   $ docker run -v /local/<USER>/ws/src/github.com/pensando/sw/:/sw \
#          -it registry.test.pensando.io:5000/pensando/drivers-linux:<VERSION>
# - Run the test
#   # cd /sw
#   # ./platform/tools/drivers-linux-build-test.sh
#
# Updating the docker
#
# INSIDE docker:
# - Preserve older headers
#   # yumdb set installonly keep kernel-ml-devel.x86_64
# - Install new headers
#   # yum --enablerepo=elrepo-kernel install kernel-ml-devel
#   # sync
#
# OUTSIDE docker:
# - Create the new image
#   $ docker ps | grep drivers-linux -> get OLDHASH
#   $ docker pause <OLDHASH>
#   $ docker commit <OLDHASH>        -> get NEWHASH
#   $ docker tag <NEWHASH> \
#          registry.test.pensando.io:5000/pensando/drivers-linux:<VERSION+1>
#   $ docker push \
#          registry.test.pensando.io:5000/pensando/drivers-linux:<VERSION+1>
#   $ docker unpause <OLDHASH>
#
# Notes
# - The pause/unpause might not be necessary; I'm not sure.
# - Installing something other than elrepo-kernel latest? Good luck!
# - OFA headers require matching kernel headers. The easiest thing might be
#   to install the kernel headers from wherever you got the OFA headers, even
#   if it duplicates a major version already in the docker. (This is why there
#   are multiple versions of 4.19 in the docker.)

# Script details
#
# usage: drivers-linux-build-test.sh [ what ... ]
#
# If option 'what' is not specified, build test kernel space drivers with all
# kernel ofed headers found under /usr/src, and the user space driver.
#
#   drivers-linux-build-test.sh
#
# Search for headers in a location besides /usr/src by overriding SRC.
#
#   SRC=/local/allenbh/docker/drivers-linux/sysroot/usr/src \
#       drivers-linux-build-test.sh
#
# If option 'what' is 'rdma-core', only build test the user space driver.
#
#   drivers-linux-build-test.sh rdma-core
#
# If option 'what' is a path to kernel or ofed headers, only build test kernel
# space drivers with those headers.
#
#   drivers-linux-build-test.sh /usr/src/linux-headers-4.15.0-46-generic
#
#   drivers-linux-build-test.sh /usr/src/ofa_kernel/4.9.0-3-amd64
#
#   drivers-linux-build-test.sh /local/allenbh/linux
#
# More than one 'what' can be specified.
#
#   drivers-linux-build-test.sh rdma-core \
#       /usr/src/linux-headers-4.15.0-46-generic \
#       /usr/src/ofa_kernel/4.9.0-3-amd64
#

TOP=$(readlink -f "$(dirname "$0")/../..")
MARK='################################'

echo -e "\n$MARK\ndrivers package\n$MARK\n"
"$TOP/platform/tools/drivers-linux.sh" || exit
cd "$TOP/platform/gen/drivers-linux" || exit

fail() { FAILURES="$FAILURES\n  $*" ; }

: ${SRC:=/usr/src}

export KSRC=
export OFA_KSRC=

is_linux() {
    test -e "$KSRC/include/generated/autoconf.h" -a \
        -e "$KSRC/include/config/auto.conf"
}

is_ofa_1() {
    KSRC="$SRC/linux-headers-$(basename "$OFA_KSRC")"
    echo $KSRC
    is_linux
}

is_ofa_2() {
    KSRC="$SRC/kernels/$(basename "$OFA_KSRC")"
    echo $KSRC
    is_linux
}

test_linux() {
    echo -e "\n$MARK\n$TAG kernel\n$MARK\n"
    cd drivers
    make clean && make -k || fail "$TAG kernel"
    cd ..
}

test_krping() {
    echo -e "\n$MARK\n$TAG krping\n$MARK\n"
    cd krping
    make clean && make || fail "$TAG krping"
    cd ..
}

test_rdmacore() {
    echo -e "\n$MARK\nrdma-core\n$MARK\n"
    cd rdma-core
    ./build.sh || fail "rdma-core"
    cd ..
}

if [ $# -eq 0 ] ; then
    # build-test for each kernel version that has installed headers
    for KSRC in ${@:-$SRC/linux*/ $SRC/kernels/*/} ; do
        TAG=$KSRC
        is_linux && test_linux && test_krping
    done

    # build-test for each ofed rdma stack
    for OFA_KSRC in ${@:-$SRC/ofa_kernel-*/*} ; do
        TAG=$OFA_KSRC
        is_ofa_1 && test_linux && test_krping
        is_ofa_2 && test_linux && test_krping
    done

    # build-test the user space driver
    test_rdmacore
else
    # build-test each user supplied parameter
    for TAG in "$@" ; do

        # build-test the user space driver
        if [ "x$TAG" = "xrdma-core" ] ; then
            test_rdmacore
            continue
        fi

        # build-test user supplied kernel version
        KSRC="$TAG"
        if is_linux ; then
            test_linux
            test_krping
            continue
        fi

        # build-test user supplied ofed rdma stack
        OFA_KSRC="$TAG"
        if is_ofa_1 ; then
            test_linux
            test_krping
            continue
        fi
        if is_ofa_2 ; then
            test_linux
            test_krping
            continue
        fi

        # not found something to be tested
        fail "$TAG (not found)"
    done
fi

echo -e "\n$MARK\nsummary\n$MARK\n"

if [ -v FAILURES ] ; then
echo -e "failures:$FAILURES"
exit 1
fi

echo "success"
