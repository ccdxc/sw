# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET = block_list_test.gtest
MODULE_SOLIBS = block_list trace logger hal_mock mtrack list shmmgr
MODULE_LDLIBS = rt
include ${MKDEFS}/post.mk
