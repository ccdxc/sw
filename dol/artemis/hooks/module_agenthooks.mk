# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_PIPELINE = artemis
MODULE_TARGET = libdolagenthooks.lib
MODULE_SRCS   = $(shell find ${MODULE_SRC_DIR} -type f -name 'agenthooks.cc')
include ${MKDEFS}/post.mk
