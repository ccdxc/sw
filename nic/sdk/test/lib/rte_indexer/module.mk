# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = rte_indexer_test.gtest
MODULE_SOLIBS   = rte_indexer logger
MODULE_ARCH     = x86_64
include ${MKDEFS}/post.mk


