# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = coremgr.bin
MODULE_PIPELINE = iris apollo apulu artemis athena
MODULE_PREREQS  = libev.export
MODULE_LDLIBS   = z
ALL_CC_FILES    = $(wildcard ${MODULE_SRC_DIR}/*.cpp)
ALL_TEST_FILES  = $(wildcard ${MODULE_SRC_DIR}/*_test.cpp)
MODULE_SRCS     = $(filter-out $(ALL_TEST_FILES), $(ALL_CC_FILES))
include ${MKDEFS}/post.mk
