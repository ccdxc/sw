# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = libfte_base_test.so
MODULE_FWTYPE   = full
MODULE_PIPELINE = iris
MODULE_SRCS     = ${MODULE_SRC_DIR}/fte_base_test.cc
include ${MKDEFS}/post.mk
