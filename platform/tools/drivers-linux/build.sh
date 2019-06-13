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

make -j -C drivers || exit

#
# build krping
#

make -j -C krping || exit

#
# build rdma-core
#

cd rdma-core
EXTRA_CMAKE_FLAGS='-DCMAKE_BUILD_TYPE=RelWithDebInfo' \
./build.sh || exit
cd -

#
# build perftest
#

cd perftest

./autogen.sh || exit

CFLAGS="-std=gnu99 -I$DIR/rdma-core/build/include" \
LDFLAGS="-L$DIR/rdma-core/build/lib -Wl,-R$DIR/rdma-core/build/lib" \
./configure || exit

make -j || exit

cd -

#
# build qperf
#

cd qperf

./autogen.sh || exit

CFLAGS="-I$DIR/rdma-core/build/include" \
LDFLAGS="-L$DIR/rdma-core/build/lib -Wl,-R$DIR/rdma-core/build/lib" \
./configure || exit

make -j || exit

cd -
