# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = gft16_test.gtest
MODULE_PIPELINE = gft16
MODULE_SOLIBS   = ${NIC_${PIPELINE}_P4PD_SOLIBS} \
                  ${NIC_HAL_PD_SOLIBS_${ARCH}} \
                  pal pack_bytes \
                  sdk_asicrw_if sdkasicpd \
                  ${NIC_SDK_SOLIBS} \
                  bm_allocator bitmap  \
                  sdk${ASIC} sdkp4 sdkp4utils sdkxcvrdriver
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} \
                  ${NIC_THIRDPARTY_SSL_LDLIBS} \
                  ${SDK_THIRDPARTY_CAPRI_LDLIBS} \
                  ${NIC_CAPSIM_LDLIBS} \
                  ${NIC_COMMON_LDLIBS}
include ${MKDEFS}/post.mk
