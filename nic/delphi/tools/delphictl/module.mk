# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk

MODULE_TARGET   = delphictl.bin
MODULE_INCS     = ${BLD_GEN_DIR}/proto
MODULE_SOLIBS   = delphisdk halproto upgradeproto nicmgrproto commonproto sdkpal logger
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} rt ev dl
MODULE_ARLIBS   = delphishm
ALL_CC_FILES    = $(wildcard ${MODULE_SRC_DIR}/*.cc)
ALL_TEST_FILES  = $(wildcard ${MODULE_SRC_DIR}/*_test.cc)
MODULE_SRCS     = $(filter-out $(ALL_TEST_FILES), $(ALL_CC_FILES))
MODULE_INCS    += ${BLD_PROTOGEN_DIR}

include ${MKDEFS}/post.mk
