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

make -j12 -C drivers || exit

#
# build krping
#

# This krping will build on some kernel versions, but needs compat work
case $(uname -r) in
# Kernel versions known to work with this krping
4.14.*|4.15.*)
make -j12 -C krping || exit
;;
# Kernel versions known incompatible with this krping: 4.9, 4.19, 4.20
# default: skip building the krping module
*)
echo 'Skipping the build of krping module'
;;
esac


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

if [ ! -a ./configure ] ; then ./autogen.sh ; fi

CFLAGS="-std=gnu99 -I$DIR/rdma-core/build/include" \
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
