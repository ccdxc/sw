# {C} Copyright 2020 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libdpdk_device.lib
MODULE_PREREQS  = pen_dpdk.submake
MODULE_LDLIBS   = dpdk numa
MODULE_INCS     = ${BLD_OUT_DIR}/pen_dpdk_submake/include
ifeq ($(ARCH), x86_64)
MODULE_FLAGS  = -march=native
endif
include ${MKDEFS}/post.mk
