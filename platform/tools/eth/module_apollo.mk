# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_ARCH     := aarch64
MODULE_TARGET   := eth_dbgtool_apollo.bin
MODULE_PIPELINE := apollo athena
MODULE_SOLIBS   := sdkpal logger sdkplatformutils shmmgr bm_allocator \
                    ${NIC_SDK_SOLIBS} \
                    sdk_asicrw_if \
                    sdkasicpd \
                    sdkp4 sdkp4utils \
                    p4pd_${PIPELINE} \
                    p4pd_${PIPELINE}_rxdma p4pd_${PIPELINE}_txdma \
                    ${NIC_HAL_PD_SOLIBS_${ARCH}} \
                    lif_mgr sdkxcvrdriver
MODULE_LDLIBS   := crypto ${NIC_COMMON_LDLIBS} \
                    ${NIC_THIRDPARTY_GOOGLE_LDLIBS} \
                    ${SDK_THIRDPARTY_CAPRI_LDLIBS} \
                    ${NIC_CAPSIM_LDLIBS}
MODULE_PREREQS  := ${PIPELINE}_rxdma.p4bin ${PIPELINE}_txdma.p4bin
include ${MKDEFS}/post.mk
