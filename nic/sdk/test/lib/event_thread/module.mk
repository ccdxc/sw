# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET = event_thread_test.gtest
MODULE_SOLIBS = event_thread thread lfq shmmgr logger
MODULE_LDLIBS = rt
MODULE_ARCH   = x86_64
include ${MKDEFS}/post.mk
