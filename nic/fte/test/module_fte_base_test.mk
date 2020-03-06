# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = libfte_base_test.lib
MODULE_FWTYPE   = full
MODULE_ARCH     = x86_64
MODULE_PIPELINE = iris
MODULE_SRCS     = ${MODULE_SRC_DIR}/fte_base_test.cc
MODULE_SOLIBS   = e2etest_lib
MODULE_LDLIBS   = tins
include ${MKDEFS}/post.mk
