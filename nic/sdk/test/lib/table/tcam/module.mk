# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
MODULE_TARGET = tcam_test.gtest
MODULE_SOLIBS = tcam indexer ht logger shmmgr p4pd_mock
MODULE_LDOPTS = -lrt
include makedefs/module.mkt
