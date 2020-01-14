# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET = operd_test.gtest
MODULE_SOLIBS = operd
MODULE_LDLIBS = rt ev
MODULE_ARCH   = x86_64
include ${MKDEFS}/post.mk
