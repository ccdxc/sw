# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libpdsfte.so
MODULE_PIPELINE = apollo artemis athena apulu
MODULE_INCS     = ${SDKDIR}/dpdk/build/include/ ${MODULE_DIR}/${PIPELINE}
MODULE_SOLIBS   = ${NIC_FTL_LIBS}
MODULE_FLAGS    = -O3
MODULE_DEFS     = -DRTE_FORCE_INTRINSICS
MODULE_PREREQS  = dpdk.submake
MODULE_LDLIBS   = dpdk
include ${MKDEFS}/post.mk
