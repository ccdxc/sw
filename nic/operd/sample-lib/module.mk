# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = liboperd-sample.so
MODULE_PIPELINE = iris gft
MODULE_SOLIBS   = 
MODULE_LDLIBS   = rt pthread
MODULE_ARLIBS   = 
ALL_CC_FILES    = $(wildcard ${MODULE_SRC_DIR}/*.cc)
ALL_TEST_FILES  = $(wildcard ${MODULE_SRC_DIR}/*_test.cc)
MODULE_SRCS     = $(filter-out $(ALL_TEST_FILES), $(ALL_CC_FILES))
include ${MKDEFS}/post.mk
