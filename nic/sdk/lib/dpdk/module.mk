# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libdpdk_device.so
MODULE_PREREQS  = dpdk.submake
#MODULE_LDLIBS   = dpdk
MODULE_LDFLAGS  = -L${SDKDIR}/dpdk/build/lib -Wl,--whole-archive,-l:libdpdk.a,--no-whole-archive
MODULE_LDLIBS   = numa
MODULE_INCS     = ${SDKDIR}/dpdk/build/include
ifeq ($(ARCH), x86_64)
    MODULE_SOLIBS     := dpdksim sdkpal logger
endif
include ${MKDEFS}/post.mk
