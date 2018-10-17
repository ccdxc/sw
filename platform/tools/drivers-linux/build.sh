#!/bin/bash

DIR=$(dirname "$0")
DIR=$(readlink -f "$DIR")

#
# prereqs for rdma-core
#

# Ubuntu/Debian
#apt-get install build-essential libtool automake autoconf cmake gcc libudev-dev libnl-3-dev libnl-route-3-dev ninja-build pkg-config valgrind

# RedHat/Fedora
#yum install libtool automake autoconf cmake gcc libnl3-devel libudev-devel make pkgconfig valgrind-devel

#
# build ionic.ko and ionic_rdma.ko
#

KCPPFLAGS=-DHAPS make -j12 -C drivers

#
# build rdma-core
#

cd rdma-core
./build.sh || exit
cd -

#
# build perftest
#

cd perftest

if [ ! -a ./configure ] ; then ./autogen.sh ; fi

CFLAGS="-I$DIR/rdma-core/build/include" \
LDFLAGS="-L$DIR/rdma-core/build/lib -Wl,-R$DIR/rdma-core/build/lib" \
./configure || exit

make -j12 || exit

cd -

#
# build qperf
#

cd qperf

if [ ! -a ./configure ] ; then ./autogen.sh ; fi

CFLAGS="-I$DIR/rdma-core/build/include" \
LDFLAGS="-L$DIR/rdma-core/build/lib -Wl,-R$DIR/rdma-core/build/lib" \
./configure || exit

make -j12 || exit

cd -
