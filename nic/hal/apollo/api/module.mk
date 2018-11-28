# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libapolloapi.so
MODULE_PIPELINE = apollo
MODULE_SRCS     = ${MODULE_SRC_DIR}/vcn.cc ${MODULE_SRC_DIR}/subnet.cc
include ${MKDEFS}/post.mk
