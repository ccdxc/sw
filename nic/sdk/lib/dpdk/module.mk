# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libdpdk_device.so
MODULE_PREREQS  = dpdk.submake
MODULE_LDLIBS   = dpdk
MODULE_INCS     = ${SDKDIR}/dpdk/build/include
include ${MKDEFS}/post.mk
