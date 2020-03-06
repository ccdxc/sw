# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET = libpenipc_ev.lib
MODULE_SOLIBS =
MODULE_LDLIBS = ev zmq
MODULE_FLAGS  = -Werror
ALL_CC_FILES  = $(wildcard ${MODULE_SRC_DIR}/*.cc)
MODULE_SRCS   = $(ALL_CC_FILES)
include ${MKDEFS}/post.mk
