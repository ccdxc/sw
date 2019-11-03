#!/bin/sh

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <pipeline>"
    exit -1
fi

PIPELINE=$1
NICDIR=/sw/nic
VPPDIR=$NICDIR/vpp
VPP_PKG_DIR=$NICDIR/sdk/third-party/vpp-pkg/x86_64
export LD_LIBRARY_PATH=$VPP_PKG_DIR/lib:$NICDIR/build/x86_64/$PIPELINE/lib
source $VPPDIR/tools/setup-vpp-prerequisites.sh $NICDIR $PIPELINE
gdb --args $VPP_PKG_DIR/bin/vpp -c $VPPDIR/conf/vpp_1_worker_sim_gdb.conf
