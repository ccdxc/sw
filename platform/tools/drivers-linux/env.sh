#!/bin/false
# This script should be sourced, not executed

#
# Set PATH to find rdma applications in this package.
#
# Synopsis:
#
# . path/to/drivers-linux/env.sh
# ibv_devinfo
#

PKG_TOP=$(readlink -f "$(dirname "${BASH_SOURCE[0]}")")

# Previous path, script can be sourced many times and not keep adding to path
: ${OLD_PATH:="$PATH"}

# Search package before original $PATH, in case rdma apps are also installed
export PATH="$PKG_TOP/rdma-core/build/bin:$PKG_TOP/perftest:$PKG_TOP/qperf:$OLD_PATH"
