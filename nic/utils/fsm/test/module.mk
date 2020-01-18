# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = fsm_test.gtest
MODULE_PIPELINE = gft iris
MODULE_SOLIBS   = fsm trace logger thread hal_mock \
                  list twheel slab \
                  shmmgr haltestutils haltrace penipc haldelphi ${NIC_HAL_GTEST_SOLIBS}
MODULE_LDLIBS   = rt ${NIC_HAL_GTEST_LDLIBS}
MODULE_ARCH     = x86_64
include ${MKDEFS}/post.mk
