# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = agent_api_test.gtest
MODULE_PIPELINE = iris gft
MODULE_ARCH     = x86_64
MODULE_SOLIBS   = ${NIC_HAL_GTEST_SOLIBS} e2etest_lib ipc
MODULE_LDLIBS   = ${NIC_HAL_GTEST_LDLIBS} tins
MODULE_LIBS     = ${CONFIG_GTEST_WOM_LIBS}
include ${MKDEFS}/post.mk
