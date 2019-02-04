# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_ARCH     := x86_64
MODULE_PIPELINE := iris
MODULE_TARGET   := hal_api_filter_test.gtest
#MODULE_SOLIBS   := halproto indexer logger nicmgr evutils delphisdk \
#	               pciemgr_if pciemgr pciemgrutils pciehdevices pcietlp cfgspace
MODULE_SOLIBS   := nicmgr nicmgr_sysmgr nicmgr_upgrade hal_api mnet \
                    pciemgr_if pciemgr pciemgrutils pciehdevices pcietlp cfgspace \
                    intrutils pal misc evutils \
                    ${NIC_SDK_SOLIBS} trace mtrack bm_allocator \
                    commonproto nicmgrproto halproto \
                    delphisdk upgrade upgrade_app upgradeutils sysmgr \
                    sdkcapri_csrint sdkcapri_asicrw_if \
                    sdkasicpd pdcapri pdcommon hal_mock \
                    sdkp4 sdkp4utils p4pd_common_p4plus_rxdma sdkxcvrdriver \
                    p4pd_common_p4plus_txdma p4pd_${PIPELINE} \
                    asicpd ${NIC_HAL_PD_SOLIBS_${ARCH}}
#MODULE_LDLIBS   := ev ${NIC_COMMON_LDLIBS} ${NIC_THIRDPARTY_GOOGLE_LDLIBS}
MODULE_LDLIBS   := crypto ${NIC_COMMON_LDLIBS} \
                     ${NIC_THIRDPARTY_GOOGLE_LDLIBS} \
                     ${NIC_CAPSIM_LDLIBS}
MODULE_INCS     := ${BLD_PROTOGEN_DIR}/ ${MODULE_SRC_DIR}/../../../../app/nicmgrd/src
include ${MKDEFS}/post.mk
