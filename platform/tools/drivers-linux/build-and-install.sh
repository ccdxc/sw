#!/bin/bash

DIR=$(dirname "$0")
DIR=$(readlink -f "$DIR")

#
# prereqs for rdma-core: ./setup_libs.sh
#

#
# build ionic.ko and ionic_rdma.ko
# (these are not installed, so a problem will not break rebooting the host)
#
UNAME=$(uname -r)
case $UNAME in
    *.el7uek.*)
        # Oracle Unbreakable Enterprise Kernel requires special handling
        echo "Adapting to $UNAME..."
        if ! grep -q IB_PORT_IP_BASED_GIDS /usr/src/kernels/$UNAME/include/rdma/ib_verbs.h ; then
            patch -f -p1 < "$DIR/patches/ionic_rdma-uek-ip-gids.patch"
        fi
    ;;
    *)
    ;;
esac

make -j -C drivers || exit

#
# build krping
#

make -j -C krping || exit

#
# build rdma-core
# and install
#

cd rdma-core
rm -rf build
mkdir -p build
cd build
cmake -GNinja -DCMAKE_INSTALL_PREFIX:PATH=/usr ..
ninja || exit
ninja install || exit
cd ../..

# Clean out refs to drivers which have been removed
rm -f /etc/libibverbs.d/nes.driver
rm -f /etc/libibverbs.d/cxgb3.driver

#
# build perftest
# and install
#

cd perftest

./autogen.sh || exit

CFLAGS="-std=gnu99" \
./configure --prefix=/usr || exit
make -j || exit
make install

cd -

#
# build qperf
# and install
#

cd qperf

./autogen.sh || exit

./configure --prefix=/usr || exit
make -j || exit
make install

cd -

#
# Other stuff
#

cp show_gid /usr/bin
cp collect.sh /usr/bin
