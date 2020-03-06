# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libpdsframework.lib
MODULE_SRCS = $(wildcard ${MODULE_SRC_DIR}/*.cc) $(wildcard ${MODULE_SRC_DIR}/${PIPELINE}/*.cc)
MODULE_PIPELINE = apollo artemis apulu athena
include ${MKDEFS}/post.mk
