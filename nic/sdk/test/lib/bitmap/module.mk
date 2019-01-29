# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_PIPELINE = iris gft
MODULE_TARGET = bitmap_test.gtest
MODULE_SOLIBS = bitmap
MODULE_ARCH   = x86_64
include ${MKDEFS}/post.mk
