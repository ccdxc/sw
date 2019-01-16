# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = memhash_test.gtest
MODULE_PIPELINE = iris
MODULE_SOLIBS   = ${NIC_HAL_GTEST_SOLIBS} memhash \
                  crc_fast jenkins_spooky memhashp4pd_mock
MODULE_LDLIBS   = ${NIC_HAL_GTEST_LDLIBS}
MODULE_ARCH     = x86_64
include ${MKDEFS}/post.mk
