# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = list_test.gtest
MODULE_SOLIBS   = list
MODULE_LDLIBS   = pthread
include ${MKDEFS}/post.mk
