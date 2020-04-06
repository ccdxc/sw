# {C} Copyright 2020 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = logger_test.gtest
MODULE_SOLIBS   = logger utils
MODULE_ARCH     = x86_64
include ${MKDEFS}/post.mk
