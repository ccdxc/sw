# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = sysmgr.bin
MODULE_PIPELINE = iris apollo apulu artemis
ifeq (${PIPELINE}, iris)
MODULE_SOLIBS   = pal operd delphisdk eventproto eventtypes events_recorder
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} rt ev pthread z dl
MODULE_ARLIBS   = sysmgrproto delphiproto
ALL_CC_FILES    = $(wildcard ${MODULE_SRC_DIR}/*.cpp \
	                     ${MODULE_SRC_DIR}/iris/*.cpp)
ALL_TEST_FILES  = $(wildcard ${MODULE_SRC_DIR}/*_test.cpp)
else
MODULE_SOLIBS   = pal logger
MODULE_LDLIBS   = rt ev pthread z dl operd
MODULE_ARLIBS   =
ALL_CC_FILES    = $(wildcard ${MODULE_SRC_DIR}/*.cpp \
	                     ${MODULE_SRC_DIR}/apollo/*.cpp)
ALL_TEST_FILES  = $(wildcard ${MODULE_SRC_DIR}/*_test.cpp)
endif
MODULE_SRCS     = $(filter-out $(ALL_TEST_FILES), $(ALL_CC_FILES))
include ${MKDEFS}/post.mk
