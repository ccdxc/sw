#!/bin/bash

DIR=$(dirname "$0")
DIR=$(readlink -f "$DIR")
THIRD_PARTY=$DIR/src/third-party
THIRD_PARTY_RDMA=$THIRD_PARTY/rdma

#
# prereqs for rdma-core
#

#sudo apt-get install build-essential libtool automake autoconf cmake gcc libudev-dev libnl-3-dev libnl-route-3-dev ninja-build pkg-config valgrind || exit

#sudo yum install libtool automake autoconf cmake gcc libnl3-devel libudev-devel make pkgconfig valgrind-devel || exit

#
# build ionic.ko and ionic_rdma.ko
#

KCPPFLAGS=-DHAPS make -j12 -C drivers/linux

#
# build rdma-core
#

cd $THIRD_PARTY_RDMA/rdma-core
./build.sh || exit
cd -

#
# build perftest
#

cd $THIRD_PARTY_RDMA/perftest

if [ ! -a ./configure ] ; then ./autogen.sh ; fi

CFLAGS="-I$THIRD_PARTY_RDMA/rdma-core/build/include" \
LDFLAGS="-L$THIRD_PARTY_RDMA/rdma-core/build/lib -Wl,-R$THIRD_PARTY_RDMA/rdma-core/build/lib" \
./configure || exit

make -j12 || exit

cd -

#
# build qperf
#

cd $THIRD_PARTY_RDMA/qperf

if [ ! -a ./configure ] ; then ./autogen.sh ; fi

CFLAGS="-I$THIRD_PARTY_RDMA/rdma-core/build/include" \
LDFLAGS="-L$THIRD_PARTY_RDMA/rdma-core/build/lib -Wl,-R$THIRD_PARTY_RDMA/rdma-core/build/lib" \
./configure || exit

make -j12 || exit

cd -
