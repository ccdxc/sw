# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET = ipc_test.gtest
MODULE_SOLIBS = utils logger penipc
MODULE_LDLIBS = rt ev zmq
MODULE_ARCH   = x86_64
include ${MKDEFS}/post.mk
