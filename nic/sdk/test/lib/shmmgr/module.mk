# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MAKEDEFS}/pre.mk
MODULE_TARGET   = shmmgr_test.gtest
MODULE_SOLIBS   = shmmgr logger
MODULE_LDLIBS   = rt
MODULE_INCS     = ../third-party/boost/include/

include ${MAKEDEFS}/post.mk
