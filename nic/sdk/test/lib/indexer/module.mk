# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MAKEDEFS}/pre.mk
MODULE_TARGET   = indexer_test.gtest
MODULE_SOLIBS   = indexer logger
include ${MAKEDEFS}/post.mk
