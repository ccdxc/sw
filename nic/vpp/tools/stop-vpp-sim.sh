#!/bin/sh

NICDIR=$1
PIPELINE=$2
VPP_PKG_DIR=$NICDIR/sdk/third-party/vpp-pkg/x86_64

#LD_LIBRARY_PATH=$NICDIR/build/x86_64/$PIPELINE/lib:$VPP_PKG_DIR/lib $VPP_PKG_DIR/bin/vppctl show trace
#LD_LIBRARY_PATH=$NICDIR/build/x86_64/$PIPELINE/lib:$VPP_PKG_DIR/lib $VPP_PKG_DIR/bin/vppctl show logging
pkill -9 vpp_main
