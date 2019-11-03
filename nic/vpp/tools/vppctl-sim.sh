#!/bin/sh

NICDIR=/sw/nic
VPP_PKG_DIR=$NICDIR/sdk/third-party/vpp-pkg/x86_64
LD_LIBRARY_PATH=$VPP_PKG_DIR/lib $VPP_PKG_DIR/bin/vppctl "$@"
