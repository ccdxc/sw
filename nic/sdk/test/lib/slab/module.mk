# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MAKEDEFS}/pre.mk
MODULE_TARGET = slab_test.gtest
MODULE_SOLIBS = slab shmmgr logger
MODULE_LDLIBS = rt
include ${MAKEDEFS}/post.mk
