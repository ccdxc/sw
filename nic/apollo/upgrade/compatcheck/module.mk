# {C} Copyright 2020 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = pdsupgcc.bin
MODULE_PIPELINE = apulu
MODULE_INCS     = ${MODULE_GEN_DIR}
MODULE_SOLIBS   = pal pdsframework pdscore pdslearn pdsapi pdsapi_impl \
                  thread pdsapi memhash sltcam operd \
                  event_thread slhash \
		  rfc_${PIPELINE} pdsrfc \
                  ${NIC_${PIPELINE}_P4PD_SOLIBS} \
                  ${NIC_SDK_SOLIBS} ${NIC_HAL_PD_SOLIBS_${ARCH}} \
                  sdkp4 sdkp4utils sdkcapri_asicrw_if sdkcapri pdsapi_capri_impl \
                  sdkplatformutils sdkxcvrdriver sdkasicpd \
                  bm_allocator sdklinkmgr sdklinkmgrcsr memhash \
                  sltcam slhash ${NIC_${PIPELINE}_NICMGR_LIBS}
MODULE_LDLIBS   =  ${NIC_COMMON_LDLIBS} \
                   ${NIC_CAPSIM_LDLIBS} ${SDK_THIRDPARTY_CAPRI_LDLIBS} \
                   AAPL edit ncurses
include ${MKDEFS}/post.mk
