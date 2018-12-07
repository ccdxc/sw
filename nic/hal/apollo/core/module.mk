# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libocicore.so
MODULE_PIPELINE = apollo
MODULE_SRCS     = ${MODULE_SRC_DIR}/*.cc
include ${MKDEFS}/post.mk
