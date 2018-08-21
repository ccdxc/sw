# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
MODULE_TARGET = slab_test.gtest
MODULE_SOLIBS = slab shmmgr logger
MODULE_LDOPTS = -lrt
include makedefs/module.mkt
