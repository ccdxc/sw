# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libhaldelphi.lib
MODULE_PIPELINE = iris gft
MODULE_PREREQS = libhalproto.lib
ALL_CC_FILES    = $(wildcard ${MODULE_SRC_DIR}/*.cc)
ALL_TEST_FILES  = $(wildcard ${MODULE_SRC_DIR}/*_test.cc)
MODULE_SRCS     = $(filter-out $(ALL_TEST_FILES), $(ALL_CC_FILES))
MODULE_SOLIBS   = delphisdk upgrade_app halupgrade
MODULE_LDLIBS   = rt ev
include ${MKDEFS}/post.mk
