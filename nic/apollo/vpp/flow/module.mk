# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = flow_plugin.so
MODULE_PIPELINE = apollo artemis
MODULE_ARCH     = aarch64 x86_64
MODULE_PREREQS  = vpp_pkg.export
MODULE_SOLIBS   = pal pdsframework pdscore pdsapi pdsapi_impl               \
                  ${NIC_${PIPELINE}_P4PD_SOLIBS} ${NIC_SDK_SOLIBS}          \
                  sdkp4 sdkp4utils sdkcapri_asicrw_if sdkcapri              \
                  sdkplatformutils sdkxcvrdriver sdkasicpd                  \
                  rfc_${PIPELINE} pdsrfc                                    \
                  bm_allocator sdklinkmgr sdklinkmgrcsr ftlv6 ftlv4 utils   \
                  sltcam slhash ${NIC_${PIPELINE}_NICMGR_LIBS}
MODULE_LDLIBS   = ${SDK_THIRDPARTY_CAPRI_LDLIBS}                            \
                  ${NIC_COMMON_LDLIBS} ${NIC_CAPSIM_LDLIBS} dl              \
                  ${SDK_THIRD_PARTY_VPP_LIBS} AAPL
MODULE_INCS     = ${NIC_CSR_INCS}                                           \
                  ${SDK_THIRD_PARTY_VPP_INCLUDES}
MODULE_FLAGS    = ${NIC_CSR_FLAGS}
ifeq ($(ARCH),x86_64)
    MODULE_DEFS     = -O0 -DCLIB_DEBUG -DCAPRI_SW ${NIC_CSR_DEFINES} -DCLIB_LOG2_CACHE_LINE_BYTES=6
else ifeq ($(ARCH),aarch64)
    MODULE_DEFS     = -O3 -DCAPRI_SW ${NIC_CSR_DEFINES} -DCLIB_LOG2_CACHE_LINE_BYTES=6
endif
include ${MKDEFS}/post.mk
