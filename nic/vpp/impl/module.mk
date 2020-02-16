# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libpdsvpp_impl.so
MODULE_PIPELINE = apulu apollo artemis
MODULE_SRCS     = $(wildcard ${MODULE_DIR}/*.c)                             \
                  $(wildcard ${MODULE_DIR}/*.cc)                            \
                  $(wildcard ${MODULE_DIR}/${PIPELINE}/*.c)                 \
                  $(wildcard ${MODULE_DIR}/${PIPELINE}/*.cc)
MODULE_PREREQS  = vpp_pkg.export
MODULE_SOLIBS   = pal pdsframework pdscore pdsapi pdsapi_impl               \
                  ${NIC_${PIPELINE}_P4PD_SOLIBS} ${NIC_SDK_SOLIBS}          \
                  sdkp4 sdkp4utils sdkcapri_asicrw_if sdkcapri              \
                  sdkplatformutils sdkxcvrdriver sdkasicpd                  \
                  rfc_${PIPELINE} pdsrfc kvstore_lmdb                       \
                  bm_allocator sdklinkmgr sdklinkmgrcsr  utils              \
                  sltcam slhash ${NIC_${PIPELINE}_NICMGR_LIBS}              \
                  ${NIC_FTL_LIBS} pdsvpp_cfg pdsvpp_operd
MODULE_LDLIBS   = ${SDK_THIRDPARTY_CAPRI_LDLIBS}                            \
                  ${NIC_COMMON_LDLIBS} ${NIC_CAPSIM_LDLIBS} dl              \
                  ${SDK_THIRD_PARTY_VPP_LIBS} AAPL lmdb
MODULE_INCS     = ${VPP_PLUGINS_INCS} ${MODULE_DIR}/${PIPELINE}
MODULE_DEFS     = ${VPP_DEFINES_${ARCH}}
MODULE_FLAGS    = ${VPP_FLAGS_${ARCH}}
include ${MKDEFS}/post.mk
