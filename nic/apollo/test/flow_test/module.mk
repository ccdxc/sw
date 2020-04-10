# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = flow_test.gtest
MODULE_PIPELINE = apollo artemis apulu
MODULE_ARCH     = x86_64
MODULE_SOLIBS   = pal pdsframework pdscore pdsapi pdsapi_impl pdstest pdsproto \
                  ${NIC_${PIPELINE}_P4PD_SOLIBS} \
                  ${NIC_SDK_SOLIBS} ${NIC_HAL_PD_SOLIBS_${ARCH}} \
                  sdkp4 sdkp4utils sdkcapri_asicrw_if sdkcapri \
                  sdkplatformutils sdkxcvrdriver sdkasicpd kvstore_lmdb \
                  lpmitree_${PIPELINE}  rfc_${PIPELINE} pdsrfc penmetrics \
                  bm_allocator sdklinkmgr sdklinkmgrcsr ${NIC_FTL_LIBS} utils \
                  sltcam slhash ${NIC_${PIPELINE}_NICMGR_LIBS}
MODULE_LDLIBS   =  ${NIC_COMMON_LDLIBS} \
                   ${NIC_CAPSIM_LDLIBS} \
                   ${${PIPELINE}_GTEST_COMMON_LDLIBS} \
                   AAPL edit ncurses lmdb
MODULE_INCS     = ${MODULE_GEN_DIR}
MODULE_FLAGS    = ${NIC_CSR_FLAGS}
MODULE_SRCS     = $(shell find ${MODULE_SRC_DIR} -type f -name '*.cc' ! -name 'agenthooks*')
MODULE_DEFS     = -DCAPRI_SW ${NIC_CSR_DEFINES}
include ${MKDEFS}/post.mk
