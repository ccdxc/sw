# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libhalapollosvc.so
MODULE_PIPELINE = apollo
MODULE_INCS     = ${MODULE_GEN_DIR}
MODULE_SOLIBS   = pal pdsframework pdscore pdsapi pdsapi_impl \
                  ${NIC_${PIPELINE}_P4PD_SOLIBS} \
                  ${NIC_SDK_SOLIBS} ${NIC_HAL_PD_SOLIBS_${ARCH}} \
                  sdkp4 sdkp4utils sdkcapri_csrint sdkcapri_asicrw_if sdkcapri \
                  sdkplatformutils sdkxcvrdriver sdkasicpd lpmitree \
                  rfc bm_allocator sdklinkmgr sdklinkmgrcsr \
                  memhash
MODULE_LDLIBS   = ${NIC_CAPSIM_LDLIBS} AAPL
include ${MKDEFS}/post.mk
