# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   := nicmgrd.bin
MODULE_PIPELINE := iris
MODULE_PREREQS  := nicmgr.proto delphi.proto
MODULE_SOLIBS   := nicmgr hal_api mnet \
                    pciemgr_if pciemgr pciemgrutils pciehdevices pcietlp cfgspace \
                    intrutils pal misc evutils \
                    ${NIC_SDK_SOLIBS} trace mtrack bm_allocator \
                    commonproto nicmgrproto halproto \
                    delphisdk \
                    upgrade upgrade_app upgradeutils \
                    sysmgr \
                    sdkcapri_csrint sdkcapri_asicrw_if \
                    sdkasicpd pdcapri pdcommon hal_mock \
                    sdkp4 sdkp4utils sdkxcvrdriver sdkfru p4pd_${PIPELINE} \
                    p4pd_common_p4plus_rxdma p4pd_common_p4plus_txdma \
                    asicpd ${NIC_HAL_PD_SOLIBS_${ARCH}} lif_mgr devapi_iris
MODULE_LDLIBS   := crypto ${NIC_COMMON_LDLIBS} \
                    ${NIC_THIRDPARTY_GOOGLE_LDLIBS} \
                    ${NIC_CAPSIM_LDLIBS}
MODULE_INCS     := ${BLD_PROTOGEN_DIR}/ \
	               ${TOPDIR}/nic/sdk/platform/devapi

include ${MKDEFS}/post.mk
