# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = ${PIPELINE}_scale_test.gtest
MODULE_PIPELINE = apollo artemis apulu
MODULE_SOLIBS   = pal pdsframework pdscore pdsapi pdsapi_impl pdstest pdsproto \
                  ${NIC_${PIPELINE}_P4PD_SOLIBS} \
                  ${NIC_SDK_SOLIBS} ${NIC_HAL_PD_SOLIBS_${ARCH}} \
                  sdkp4 sdkp4utils sdk_asicrw_if sdk${ASIC} \
                  sdkplatformutils sdkxcvrdriver sdkasicpd sdkfru pal \
                  lpmitree_${PIPELINE} rfc_${PIPELINE} pdsrfc penmetrics \
                  bm_allocator sdklinkmgr sdklinkmgrcsr kvstore_lmdb \
                  sltcam slhash memhash ${NIC_FTL_LIBS} \
                  ${NIC_${PIPELINE}_NICMGR_LIBS}
MODULE_LDLIBS   =  ${NIC_COMMON_LDLIBS} \
                   ${NIC_CAPSIM_LDLIBS} \
                   ${${PIPELINE}_GTEST_COMMON_LDLIBS} \
                   AAPL edit ncurses lmdb
MODULE_INCS     = ${MODULE_GEN_DIR}
MODULE_FLAGS    = ${NIC_CSR_FLAGS}
MODULE_DEFS     = -DCAPRI_SW ${NIC_CSR_DEFINES}
MODULE_SRCS     = ${MODULE_SRC_DIR}/main.cc \
                  ${MODULE_SRC_DIR}/test.cc \
                  ${MODULE_SRC_DIR}/${PIPELINE}/pkt.cc \
                  ${MODULE_SRC_DIR}/api_pds.cc
include ${MKDEFS}/post.mk
