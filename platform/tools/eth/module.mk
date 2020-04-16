# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_ARCH     := aarch64
MODULE_TARGET   := eth_dbgtool.bin
MODULE_PIPELINE := iris
MODULE_SOLIBS   := sdkpal logger sdkplatformutils shmmgr bm_allocator \
                    ${NIC_SDK_SOLIBS} \
                    sdkcapri_asicrw_if \
                    sdkasicpd pdcommon\
                    sdkp4 sdkp4utils \
                    asicpd \
                    p4pd_${PIPELINE} \
                    p4pd_common_p4plus_rxdma p4pd_common_p4plus_txdma \
                    hal_mock ${NIC_HAL_PD_SOLIBS_${ARCH}} \
                    lif_mgr sdkxcvrdriver
MODULE_LDLIBS   := crypto ${NIC_COMMON_LDLIBS} \
                    ${NIC_THIRDPARTY_GOOGLE_LDLIBS} \
                    ${SDK_THIRDPARTY_CAPRI_LDLIBS} \
                    ${NIC_CAPSIM_LDLIBS}
MODULE_PREREQS  := common_p4plus_rxdma.p4bin common_p4plus_txdma.p4bin hal.memrgns
include ${MKDEFS}/post.mk
