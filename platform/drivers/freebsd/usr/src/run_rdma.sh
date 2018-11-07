#!/bin/sh
#
# Script for running rdma user space with LD_PRELOAD
#
# In freebsd, userspace driver libraries are dynamically linked dependecies of
# installed rdma user space apps like ibv_rc_pingpong.  Unless we insert
# pensando drivers in the bsd src and rebuild (make buildworld), the pensando
# drivers will not be loaded automatically.  Here, we can use LD_PRELOAD to
# instuct ld.so to load pensando drivers that would not otherwise be loaded.
#
# eg.  package-freebsd/run_rdma.sh ibv_devinfo
# eg.  package-freebsd/run_rdma.sh ib_send_bw
#
# instead of:
#
# LD_PRELOAD=/root/package-freebsd/contrib/ofed/libionic/libionic.so ibv_devinfo
#

PKG_TOP=$(readlink -f "$(dirname "$0")")

PATH="$PKG_TOP/perftest:$PATH"
IONIC_LIB="$PKG_TOP/contrib/ofed/libionic/libionic.so"

LD_PRELOAD="$IONIC_LIB" "$@"
