# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET = sldirectmap_test.gtest
MODULE_SOLIBS = rte_indexer shmmgr ht sldirectmap logger p4pd_mock
MODULE_LDLIBS = rt
MODULE_ARCH   = x86_64
include ${MKDEFS}/post.mk
