# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = hal_scale_test.gtest
MODULE_PIPELINE = iris
MODULE_ARCH     = x86_64 aarch64
MODULE_SRCS     = ${MODULE_SRC_DIR}/hal_scale_test.cc
MODULE_SOLIBS   = ${NIC_HAL_GTEST_SOLIBS}
MODULE_LDLIBS   = ${NIC_HAL_GTEST_LDLIBS}
include ${MKDEFS}/post.mk
