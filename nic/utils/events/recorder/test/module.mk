# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = events_recorder_test.gtest
MODULE_ARCH     = x86_64
MODULE_SOLIBS   = ${NIC_HAL_GTEST_SOLIBS} e2etest_lib ipc events_recorder
MODULE_LDLIBS   = ${NIC_HAL_GTEST_LDLIBS} tins
MODULE_LIBS     = ${CONFIG_GTEST_WOM_LIBS}
include ${MKDEFS}/post.mk