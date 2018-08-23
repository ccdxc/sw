# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MAKEDEFS}/pre.mk
MODULE_TARGET   = list_test.gtest
MODULE_SOLIBS   = list
MODULE_LDLIBS   = pthread
include ${MAKEDEFS}/post.mk
