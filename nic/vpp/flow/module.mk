# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET    = flow_plugin.so
MODULE_PIPELINE  = apollo artemis apulu
MODULE_SRCS      = $(wildcard ${MODULE_DIR}/*.c)                             \
                   $(wildcard ${MODULE_DIR}/*.cc)                            \
				   $(wildcard ${MODULE_DIR}/${PIPELINE}/*.c)                 \
				   $(wildcard ${MODULE_DIR}/${PIPELINE}/*.cc)
MODULE_PREREQS  = vpp_pkg.export
MODULE_SOLIBS   = logger sdkpal sdkcapri ftlv4 ftlv6                        \
                  sdkp4 sdkp4utils sdkasicpd bm_allocator                   \
                  ${NIC_SDK_SOLIBS} ${NIC_${PIPELINE}_P4PD_SOLIBS}          \
                  ${NIC_HAL_PD_SOLIBS_${ARCH}}
MODULE_LDLIBS   = ${SDK_THIRDPARTY_CAPRI_LDLIBS}                            \
                  ${NIC_COMMON_LDLIBS} ${NIC_CAPSIM_LDLIBS} dl              \
                  ${SDK_THIRD_PARTY_VPP_LIBS}
MODULE_INCS     = ${VPP_PLUGINS_INCS}                                       \
                  ${MODULE_DIR}/${PIPELINE}
MODULE_DEFS     = ${VPP_DEFINES_${ARCH}}
MODULE_FLAGS     = ${VPP_FLAGS_${ARCH}}
include ${MKDEFS}/post.mk
