# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = fte_test.gtest
MODULE_ARCH     = x86_64
MODULE_PIPELINE = iris
MODULE_SOLIBS   = ${NIC_HAL_GTEST_SOLIBS} e2etest_lib
MODULE_LDLIBS   = ${NIC_HAL_GTEST_LDLIBS} tins
MODULE_LIBS     = ${CONFIG_GTEST_WOM_LIBS}
include ${MKDEFS}/post.mk
