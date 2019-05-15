# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = pt_test.gtest
MODULE_PIPELINE = iris gft
MODULE_SOLIBS   = pt trace logger \
                  hal_mock trace mtrack \
                  slab shmmgr
MODULE_LDLIBS   = rt
MODULE_ARCH     = x86_64
include ${MKDEFS}/post.mk
