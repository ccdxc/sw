# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = arp_proxy.so
MODULE_PIPELINE = apollo artemis
MODULE_ARCH     = aarch64 x86_64
MODULE_PREREQS  = vpp_pkg.export
MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/*.c)
ifeq ($(PIPELINE), apollo)
    MODULE_SRCS     += $(wildcard ${MODULE_SRC_DIR}/${PIPELINE}/*.cc)
else
    MODULE_SRCS     += $(wildcard ${MODULE_SRC_DIR}/stub/*.cc)
endif
MODULE_SOLIBS   = sdkp4 sdkp4utils sdkasicpd sdkpal bm_allocator             \
                  ${NIC_${PIPELINE}_P4PD_SOLIBS}          		     \
                  ${NIC_SDK_SOLIBS} ${NIC_HAL_PD_SOLIBS_${ARCH}}
MODULE_LDLIBS   = ${SDK_THIRDPARTY_CAPRI_LDLIBS} ${SDK_THIRD_PARTY_VPP_LIBS} \
                  ${NIC_COMMON_LDLIBS} ${NIC_CAPSIM_LDLIBS} pthread
MODULE_INCS     = ${SDK_THIRD_PARTY_VPP_INCLUDES}
MODULE_LDPATHS  = ${SDK_THIRD_PARTY_VPP_LDPATH}
MODULE_DEFS     = -DCAPRI_SW ${NIC_CSR_DEFINES} -DCLIB_LOG2_CACHE_LINE_BYTES=6
MODULE_FLAGS    = ${NIC_CSR_FLAGS}
ifeq ($(ARCH),x86_64)
    MODULE_DEFS     = -O0 -DCLIB_DEBUG -DCAPRI_SW ${NIC_CSR_DEFINES} -DCLIB_LOG2_CACHE_LINE_BYTES=6
else ifeq ($(ARCH),aarch64)
    MODULE_DEFS     = -O3 -DCAPRI_SW ${NIC_CSR_DEFINES} -DCLIB_LOG2_CACHE_LINE_BYTES=6
endif
include ${MKDEFS}/post.mk
