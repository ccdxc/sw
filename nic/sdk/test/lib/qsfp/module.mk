# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET = qsfp_test.gtest
MODULE_SOLIBS = sdkpal
MODULE_LDLIBS = dl logger thread
MODULE_ARCH   = aarch64
include ${MKDEFS}/post.mk
