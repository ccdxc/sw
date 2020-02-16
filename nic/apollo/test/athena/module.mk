# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = athena_test.bin
MODULE_PIPELINE = athena
MODULE_ARCH     = x86_64
MODULE_INCS     = ${MODULE_GEN_DIR}
MODULE_LDFLAGS  = -L$(MS_LIB_DIR)
MODULE_SOLIBS   = pal pdsframework pdscore pdslearn pdsapi pdsapi_impl \
                  thread trace logger pdsapi memhash sltcam \
                  event_thread slhash \
                  ${NIC_${PIPELINE}_P4PD_SOLIBS} \
                  ${NIC_SDK_SOLIBS} ${NIC_HAL_PD_SOLIBS_${ARCH}} \
                  sdkp4 sdkp4utils sdkcapri_asicrw_if sdkcapri pdsapi_capri_impl \
                  sdkplatformutils sdkxcvrdriver sdkasicpd kvstore_lmdb \
                  bm_allocator sdklinkmgr sdklinkmgrcsr ${NIC_FTL_LIBS} memhash \
                  sltcam slhash ${NIC_${PIPELINE}_NICMGR_LIBS} utils
MODULE_LDLIBS   =  ${NIC_COMMON_LDLIBS} \
                   ${NIC_CAPSIM_LDLIBS} ${SDK_THIRDPARTY_CAPRI_LDLIBS} \
                   AAPL edit ncurses lmdb
include ${MKDEFS}/post.mk
