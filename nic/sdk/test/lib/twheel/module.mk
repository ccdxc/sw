# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
MODULE_TARGET = twheel_test.gtest
MODULE_SOLIBS = twheel slab shmmgr logger
MODULE_LDOPTS = -lrt
include makedefs/module.mkt
