# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET = pt_test.gtest
MODULE_SOLIBS = pt trace logger print \
                hal_mock trace mtrack \
                slab shmmgr
MODULE_LDLIBS = rt
include ${MKDEFS}/post.mk
