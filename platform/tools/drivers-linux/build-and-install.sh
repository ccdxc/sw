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
# (these are not installed, so a problem will not break rebooting the host)
#

KCPPFLAGS=-DHAPS make -j12 -C drivers

#
# build rdma-core
# and install
#

cd rdma-core
mkdir -p build
cd build
cmake -GNinja -DCMAKE_INSTALL_PREFIX:PATH=/usr ..
ninja
ninja install
cd ../..

#
# build perftest
# and install
#

cd perftest

if [ ! -a ./configure ] ; then ./autogen.sh ; fi

./configure --prefix=/usr || exit
make -j12 || exit
make install

cd -

#
# build qperf
# and install
#

cd qperf

if [ ! -a ./configure ] ; then ./autogen.sh ; fi

./configure --prefix=/usr || exit
make -j12 || exit
make install

#
# Other sutff
#

cp show_gid /usr/bin
cp collect.sh /usr/bin

cd -
