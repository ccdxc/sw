# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = net_fwding_test.gtest
MODULE_FWTYPE   = full
MODULE_ARCH     = x86_64
MODULE_PIPELINE = iris
MODULE_SRCS     = ${MODULE_SRC_DIR}/net_fwding_test.cc
MODULE_SOLIBS   = ${NIC_HAL_GTEST_SOLIBS} fte_base_test e2etest_lib
MODULE_LDLIBS   = ${NIC_HAL_GTEST_LDLIBS} tins
MODULE_LIBS     = ${CONFIG_GTEST_WOM_LIBS}
include ${MKDEFS}/post.mk
