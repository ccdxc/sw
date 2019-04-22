# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = crc_test_zlib
MODULE_PIPELINE = iris gft
MODULE_LDLIBS   = pthread z
MODULE_ARCH     = x86_64
MODULE_SRCS     = ${MODULE_SRC_DIR}/crc_test_zlib.cc
include ${MKDEFS}/post.mk
