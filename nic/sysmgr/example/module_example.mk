# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk

MODULE_TARGET   = sysmgr_example.bin
MODULE_ARCH     = x86_64
MODULE_SOLIBS   = delphisdk sysmgr
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} rt ev boost_program_options
MODULE_ARLIBS   = sysmgrproto delphiproto
ALL_CC_FILES    = $(wildcard ${MODULE_SRC_DIR}/example.cc)
ALL_TEST_FILES  = $(wildcard ${MODULE_SRC_DIR}/*_test.cpp)
MODULE_SRCS     = $(filter-out $(ALL_TEST_FILES), $(ALL_CC_FILES))

include ${MKDEFS}/post.mk
