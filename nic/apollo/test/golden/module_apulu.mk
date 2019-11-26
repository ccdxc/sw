# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = apulu_test.gtest
MODULE_PIPELINE = apulu
MODULE_ARCH     = x86_64
MODULE_SOLIBS   = ${NIC_${PIPELINE}_P4PD_SOLIBS} \
                  ${NIC_HAL_PD_SOLIBS_${ARCH}} \
                  pal pack_bytes \
                  sdkcapri_asicrw_if sdkasicpd \
                  ${NIC_SDK_SOLIBS} \
                  bm_allocator bitmap \
                  sdkcapri sdkp4 sdkp4utils sdkxcvrdriver
MODULE_LDLIBS   = ${NIC_CAPSIM_LDLIBS} \
                  ${NIC_COMMON_LDLIBS} \
                  ${${PIPELINE}_GTEST_COMMON_LDLIBS}
MODULE_SRCS     := ${MODULE_SRC_DIR}/apulu.cc
MODULE_DEPS     := ${MODULE_SRC_DIR}/apulu_pkts.h
include ${MKDEFS}/post.mk
