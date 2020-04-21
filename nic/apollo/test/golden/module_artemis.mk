# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = artemis_test.gtest
MODULE_PIPELINE = artemis
MODULE_ARCH     = x86_64
MODULE_SOLIBS   = ${NIC_${PIPELINE}_P4PD_SOLIBS} \
                  ${NIC_HAL_PD_SOLIBS_${ARCH}} \
                  pal pack_bytes \
                  sdk_asicrw_if sdkasicpd \
                  ${NIC_SDK_SOLIBS} \
                  bm_allocator bitmap \
                  sdk${ASIC} sdkp4 sdkp4utils sdkxcvrdriver
MODULE_LDLIBS   = ${NIC_CAPSIM_LDLIBS} \
                  ${NIC_COMMON_LDLIBS} \
                  ${${PIPELINE}_GTEST_COMMON_LDLIBS}
MODULE_SRCS     := ${MODULE_SRC_DIR}/artemis.cc
include ${MKDEFS}/post.mk
