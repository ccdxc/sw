# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = gft_test.gtest
MODULE_PIPELINE = gft
MODULE_SOLIBS   = ${NIC_${PIPELINE}_P4PD_SOLIBS} \
                  ${NIC_HAL_PD_SOLIBS_${ARCH}} \
                  pal hal_mock halproto \
                  sdk_asicrw_if \
                  ${NIC_SDK_SOLIBS} \
                  pdcommon fte_mock agent_api \
                  bm_allocator bitmap trace  \
                  sdk${ASIC} sdkp4 sdkp4utils \
                  sdkasicpd sdkasiccmn asicpd hal_mock hal_lib haltrace \
                  ${NIC_LINKMGR_SOLIBS}
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} \
                  ${NIC_THIRDPARTY_SSL_LDLIBS} \
                  ${SDK_THIRDPARTY_CAPRI_LDLIBS} \
                  ${NIC_CAPSIM_LDLIBS} \
                  ${NIC_COMMON_LDLIBS} \
                  ${NIC_LINKMGR_LDLIBS}

include ${MKDEFS}/post.mk
