# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = iris_c_filter_test.gtest
MODULE_PIPELINE = iris
MODULE_SRCS     = ${MODULE_SRC_DIR}/filter_test.cc
MODULE_SOLIBS   = iris_c iris_c_utils halproto logger indexer
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} z
MODULE_INCS     = ${MODULE_SRC_DIR}/../include
include ${MKDEFS}/post.mk
