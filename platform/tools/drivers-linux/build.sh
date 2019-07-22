#!/bin/bash

DIR=$(dirname "$0")
DIR=$(readlink -f "$DIR")

#
# prereqs for rdma-core: run ./setup_libs.sh
#

#
# build ionic.ko and ionic_rdma.ko
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
