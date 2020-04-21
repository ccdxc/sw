# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET = linkmgr_delphi_test.gtest
MODULE_ARCH     = x86_64
MODULE_PIPELINE = iris gft
MODULE_SRCS   = ${MODULE_SRC_DIR}/linkmgr_delphi_test.cc
MODULE_SOLIBS = ${NIC_HAL_GTEST_SOLIBS} linkmgr_test_utils linkmgr_src
MODULE_LDLIBS = ${NIC_HAL_GTEST_LDLIBS}
MODULE_FLAGS  = -pthread -rdynamic
MODULE_INCS     = ${BLD_PROTOGEN_DIR}
ifeq ($(ASIC),elba)
MODULE_FLAGS  += -DELBA
endif
include ${MKDEFS}/post.mk
