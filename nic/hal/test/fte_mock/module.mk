# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET = libfte_mock.lib
MODULE_SRCS   = ${MODULE_SRC_DIR}/fte_mock.cc
MODULE_PIPELINE = iris gft
MODULE_ARCH   = x86_64
include ${MKDEFS}/post.mk
