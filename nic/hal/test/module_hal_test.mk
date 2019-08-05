# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = hal_test.gtest
MODULE_PIPELINE = iris
MODULE_ARCH     = x86_64 aarch64
MODULE_SRCS     = ${MODULE_SRC_DIR}/hal_test.cc ${MODULE_SRC_DIR}/fips_rsa_testvec_parser.cc ${MODULE_SRC_DIR}/fips_sha3_testvec_parser.cc
MODULE_SOLIBS   = ${NIC_HAL_GTEST_SOLIBS}
MODULE_LDLIBS   = ${NIC_HAL_GTEST_LDLIBS}
include ${MKDEFS}/post.mk
