# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = gft_scale_test.gtest
MODULE_PIPELINE = gft
MODULE_SOLIBS   = ${NIC_HAL_GTEST_SOLIBS}
MODULE_LDLIBS   = ${NIC_HAL_GTEST_LDLIBS}
include ${MKDEFS}/post.mk
