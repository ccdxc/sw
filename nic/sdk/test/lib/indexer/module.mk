# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = indexer_test.gtest
MODULE_SOLIBS   = indexer logger
MODULE_ARCH     = x86_64
include ${MKDEFS}/post.mk
