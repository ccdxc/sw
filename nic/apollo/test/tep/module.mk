# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = apollo_tep_test.gtest
MODULE_PIPELINE = apollo
MODULE_SOLIBS   = pal pdsframework pdscore pdsapi pdsapi_impl pdstestutils \
                  ${NIC_${PIPELINE}_P4PD_SOLIBS} \
                  ${NIC_SDK_SOLIBS} ${NIC_HAL_PD_SOLIBS_${ARCH}} \
                  sdkp4 sdkp4utils sdkcapri_csrint sdkcapri_asicrw_if sdkcapri \
                  sdkplatformutils sdkxcvrdriver sdkasicpd lpmitree \
                  bm_allocator sdklinkmgr sdklinkmgrcsr memhash \
                  ${NIC_APOLLO_NICMGR_LIBS}
MODULE_LDLIBS   =  ${NIC_COMMON_LDLIBS} ${NIC_CAPSIM_LDLIBS} \
                   AAPL
MODULE_INCS     = ${NIC_CSR_INCS}
MODULE_FLAGS    = ${NIC_CSR_FLAGS}
MODULE_DEFS     = -DCAPRI_SW ${NIC_CSR_DEFINES}
include ${MKDEFS}/post.mk
