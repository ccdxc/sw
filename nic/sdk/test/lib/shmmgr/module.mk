# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = shmmgr_test.gtest
MODULE_SOLIBS   = shmmgr logger
MODULE_LDLIBS   = rt
include ${MKDEFS}/post.mk
