# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = arp_proxy.so
MODULE_PIPELINE = apollo artemis apulu
MODULE_ARCH     = aarch64 x86_64
MODULE_PREREQS  = vpp_pkg.export
MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/*.c)
MODULE_SOLIBS   = sdkp4 sdkp4utils sdkasicpd sdkpal bm_allocator             \
                  ${NIC_${PIPELINE}_P4PD_SOLIBS}                             \
                  ${NIC_SDK_SOLIBS} ${NIC_HAL_PD_SOLIBS_${ARCH}}
MODULE_INCS	= ${VPP_PLUGINS_INCS} ${MODULE_DIR}
ifeq ($(PIPELINE), apulu)
MODULE_SRCS     += $(wildcard ${MODULE_SRC_DIR}/${PIPELINE}/*.cc)
MODULE_SOLIBS   += pdsvpp_impl
MODULE_INCS     += ${MODULE_DIR}/../impl/${PIPELINE}/
else
MODULE_INCS     += ${MODULE_DIR}/../impl/stub/
endif
MODULE_LDLIBS   = ${SDK_THIRDPARTY_CAPRI_LDLIBS} ${SDK_THIRD_PARTY_VPP_LIBS} \
                  ${NIC_COMMON_LDLIBS} ${NIC_CAPSIM_LDLIBS} pthread
MODULE_LDPATHS  = ${SDK_THIRD_PARTY_VPP_LDPATH}
MODULE_DEFS     = ${VPP_DEFINES_${ARCH}}
MODULE_FLAGS    = ${VPP_FLAGS_${ARCH}}
include ${MKDEFS}/post.mk
