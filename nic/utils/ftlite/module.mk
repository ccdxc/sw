# {C} Copyright 2019 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET = libftlite.so
MODULE_SOLIBS = pal pdsframework pdscore pdsapi pdsapi_impl pdstestutils \
				${NIC_${PIPELINE}_P4PD_SOLIBS} \
				${NIC_SDK_SOLIBS} ${NIC_HAL_PD_SOLIBS_${ARCH}} \
				sdkp4 sdkp4utils sdkcapri_asicrw_if sdkcapri \
				sdkplatformutils sdkxcvrdriver sdkasicpd lpmitree_${PIPELINE} \
				bm_allocator sdklinkmgr sdklinkmgrcsr memhash utils \
				${NIC_APOLLO_NICMGR_LIBS}
MODULE_LDLIBS = ${NIC_COMMON_LDLIBS} ${NIC_CAPSIM_LDLIBS} \
                ${SDK_THIRDPARTY_CAPRI_LDLIBS} AAPL
ifeq ($(PLATFORM),hw)
MODULE_FLAGS  = -O3 -DUSE_ARM64_SIMD
else
MODULE_FLAGS  = -O3
endif
MODULE_PIPELINE = artemis
include ${MKDEFS}/post.mk
