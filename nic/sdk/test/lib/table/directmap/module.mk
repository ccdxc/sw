# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
MODULE_TARGET = directmap_test.gtest
MODULE_SOLIBS = indexer shmmgr ht directmap logger p4pd_mock
MODULE_LDOPTS = -lrt
include makedefs/module.mkt
