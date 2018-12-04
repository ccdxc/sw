# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk

MODULE_TARGET   = penlog_test.bin
MODULE_ARCH     = x86_64
MODULE_SOLIBS   = delphisdk penlog
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} rt ev pthread
MODULE_ARLIBS   = delphiproto penlogproto
ALL_CC_FILES    = $(wildcard ${MODULE_SRC_DIR}/*.cpp)
ALL_TEST_FILES  = $(wildcard ${MODULE_SRC_DIR}/*_test.cpp)
MODULE_SRCS     = $(filter-out $(ALL_TEST_FILES), $(ALL_CC_FILES))

include ${MKDEFS}/post.mk
