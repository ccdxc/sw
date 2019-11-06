#!/bin/bash

# Load eth and rdma modules
#
# ./load.sh [extra module parameters]
#
# Usually just:
# ./load.sh
#
# Extra debugging:
# ./load.sh dyndbg=+pmfl
#
# Preferably any workarounds, if needed, should be enabled by default in
# drivers themselves.  Nondefault options can be passed to specific drivers in
# XXX_ETH or XXX_RDMA variables, or via extra module parameters on the command
# line.  Note that if only one module recognizes a parameter on the command
# line, the other will warn, but the warning is harmless.

PKG_TOP=$(readlink -f "$(dirname "$0")")

modprobe ib_uverbs
modprobe rdma_ucm
insmod "$PKG_TOP/krping/rdma_krping.ko" "$@"

insmod "$PKG_TOP/drivers/eth/ionic/ionic.ko" $XXX_ETH "$@"
insmod "$PKG_TOP/drivers/rdma/drv/ionic/ionic_rdma.ko" $XXX_RDMA "$@"
