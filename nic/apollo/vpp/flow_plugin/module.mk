# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = flow_plugin.so
MODULE_PIPELINE = apollo
MODULE_ARCH     = aarch64
#MODULE_PREREQS  = vpp_build.export
MODULE_SOLIBS   = pal pdsframework pdscore pdsapi pdsapi_impl pdstestutils  \
                  ${NIC_${PIPELINE}_P4PD_SOLIBS}                            \
                  ${NIC_SDK_SOLIBS} ${NIC_HAL_PD_SOLIBS_${ARCH}}            \
                  sdkp4 sdkp4utils sdkcapri_asicrw_if sdkcapri              \
                  sdkplatformutils sdkxcvrdriver sdkasicpd lpmitree         \
                  bm_allocator sdklinkmgr sdklinkmgrcsr memhash utils ftl   \
                  ${NIC_APOLLO_NICMGR_LIBS}
MODULE_LDLIBS   =  ${NIC_COMMON_LDLIBS}                                     \
                   ${NIC_CAPSIM_LDLIBS}                                     \
                   ${SDK_THIRDPARTY_CAPRI_LDLIBS}                           \
                   ${SDK_THIRD_PARTY_VPP_LIBS}                              \
                   AAPL
MODULE_INCS     = ${NIC_CSR_INCS}                                           \
                  ${SDK_THIRD_PARTY_VPP_INCLUDES}
MODULE_LDPATHS  = ${SDK_THIRD_PARTY_VPP_LDPATH}
MODULE_FLAGS    = ${NIC_CSR_FLAGS}
MODULE_DEFS     = -DCAPRI_SW ${NIC_CSR_DEFINES} -DCLIB_LOG2_CACHE_LINE_BYTES=7
include ${MKDEFS}/post.mk
