#!/bin/bash
#
# Script for running rdma user space with PATH
#
# FreeBSD adds run_rdma.sh to set LD_PRELOAD only for rdma programs.
# Similarly, the linux package also provides run_rdma.sh so that test commands
# can be the same between linux and freebsd.  It is not required to source
# env.sh prior to the command when run with this run_rdma.sh script.
#
# Synopsis:
#
# drivers-linux/run_rdma.sh [command]
#
# Eg:
#
# drivers-linux/run_rdma.sh ibv_devinfo
#

PKG_TOP=$(readlink -f "$(dirname "$0")")

cd "$PKG_TOP"
. env.sh
cd -

"$@"
