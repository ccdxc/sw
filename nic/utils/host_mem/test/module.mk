# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET = host_mem_test.gtest
MODULE_SOLIBS = host_mem trace logger \
                hal_mock mtrack \
                bm_allocator
include ${MKDEFS}/post.mk
