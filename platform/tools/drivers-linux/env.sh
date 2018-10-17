#!/bin/false
# This script should be sourced, not executed

#
# Set PATH to find rdma applications in this package.
#
# This script should be run from the drivers-linux dir.
#
# Synopsis:
#
# cd path/to/drivers-linux
# . env.sh
#

# Previous path, script can be sourced many times and not keep adding to path
: ${OLD_PATH:="$PATH"}

# Search package before original $PATH, in case rdma apps are also installed
export PATH="$PWD/rdma-core/build/bin:$PWD/perftest:$PWD/qperf:$OLD_PATH"
