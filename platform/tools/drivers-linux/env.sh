#!/bin/false
# This script should be sourced, not executed

#
# Set PATH to find rdma applications in this package.
#
# Synopsis:
#
# cd path/to/drivers
# . env.sh
# ibv_devinfo
#

# Previous path, script can be sourced many times and not keep adding to path
: ${OLD_PATH:="$PATH"}

# Search package before original $PATH, in case rdma apps are also installed
export PATH="$PWD/rdma-core/build/bin:$PWD/perftest:$PWD/qperf:$OLD_PATH"
