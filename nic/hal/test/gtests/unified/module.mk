# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = unify_test.gtest
MODULE_PIPELINE = iris
MODULE_SOLIBS   = ${NIC_HAL_GTEST_SOLIBS}
MODULE_LDLIBS   = ${NIC_HAL_GTEST_LDLIBS}
MODULE_ARCH     = x86_64
include ${MKDEFS}/post.mk
