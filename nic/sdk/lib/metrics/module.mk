# {C} Copyright 2020 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libpenmetrics.so
MODULE_PIPELINE = iris apollo apulu gft artemis
MODULE_SOLIBS   = sdkpal logger
MODULE_LDLIBS   = dl
MODULE_FLAGS    = -Werror -Wall
ALL_CC_FILES    = $(wildcard ${MODULE_SRC_DIR}/*.cc)
ALL_TEST_FILES  = $(wildcard ${MODULE_SRC_DIR}/*test*.cc)
MODULE_SRCS     = $(filter-out $(ALL_TEST_FILES), $(ALL_CC_FILES))
include ${MKDEFS}/post.mk
