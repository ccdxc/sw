# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET = sdkfru_test.gtest
MODULE_SOLIBS = pal sdkfru
MODULE_LDLIBS = dl
MODULE_ARCH   = aarch64
include ${MKDEFS}/post.mk
