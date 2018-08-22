# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
MODULE_TARGET   = hash_test.gtest
MODULE_SOLIBS   = logger shmmgr ht tcam hash p4pd_mock indexer
MODULE_LDOPTS   = -lrt
MODULE_INCS     = ../third-party/boost/include/
include makedefs/module.mkt
