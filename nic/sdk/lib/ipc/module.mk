# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET = libpenipc.lib
MODULE_SOLIBS =
MODULE_LDLIBS = zmq
MODULE_FLAGS  = -Werror
ALL_CC_FILES  = $(wildcard ${MODULE_SRC_DIR}/*.cc)
EV_CC_FILES   = $(wildcard ${MODULE_SRC_DIR}/*_ev.cc)
MODULE_SRCS   = $(filter-out $(EV_CC_FILES), $(ALL_CC_FILES))
include ${MKDEFS}/post.mk
