# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = dsc_ssl_client.gtest
MODULE_PIPELINE = iris
MODULE_ARCH     = x86_64 aarch64
MODULE_SRCS     = ${MODULE_SRC_DIR}/dsc_client.cc
MODULE_SOLIBS   = utils sslutils
MODULE_LDLIBS   = ssl crypto
include ${MKDEFS}/post.mk
