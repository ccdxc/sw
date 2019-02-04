# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = apollo_vcn_test.gtest
MODULE_PIPELINE = apollo
MODULE_SOLIBS   = pal ociframework ocicore ociapi ociapi_impl ocitestutils \
                  ${NIC_${PIPELINE}_P4PD_SOLIBS} \
                  ${NIC_SDK_SOLIBS} ${NIC_HAL_PD_SOLIBS_${ARCH}} \
                  sdkp4 sdkp4utils sdkcapri_csrint sdkcapri_asicrw_if sdkcapri \
                  sdkplatformutils sdkxcvrdriver sdkasicpd lpmitree \
                  bm_allocator mtrack sdklinkmgr sdklinkmgrcsr memhash
MODULE_LDLIBS   = ${NIC_COMMON_LDLIBS} ${NIC_CAPSIM_LDLIBS} \
		  ${NIC_THIRDPARTY_SSL_LDLIBS} AAPL
include ${MKDEFS}/post.mk
