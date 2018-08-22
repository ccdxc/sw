# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
MODULE_TARGET   = shmmgr_test.gtest
MODULE_SOLIBS   = shmmgr logger
MODULE_LDOPTS   = -lrt
MODULE_INCS     = ../third-party/boost/include/
include makedefs/module.mkt
