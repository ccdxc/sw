# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = hbm_hash_flow_table_test.gtest
MODULE_PIPELINE = iris
MODULE_ARCH     = x86_64
MODULE_SOLIBS   = ${NIC_HAL_GTEST_SOLIBS}
MODULE_LDLIBS   = ${NIC_HAL_GTEST_LDLIBS}
include ${MKDEFS}/post.mk
