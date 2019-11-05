# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libsysmgr.so
MODULE_PIPELINE = iris gft
MODULE_SOLIBS   = penlog
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} rt ev pthread
MODULE_ARLIBS   = sysmgrproto delphiproto
ALL_CC_FILES    = $(wildcard ${MODULE_SRC_DIR}/*.cpp)
ALL_TEST_FILES  = $(wildcard ${MODULE_SRC_DIR}/*_test.cpp)
MODULE_SRCS     = $(filter-out $(ALL_TEST_FILES), $(ALL_CC_FILES))
include ${MKDEFS}/post.mk
