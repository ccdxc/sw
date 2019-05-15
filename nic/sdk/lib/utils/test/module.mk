# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = crc_test.bin
MODULE_LDLIBS   = pthread
MODULE_ARCH     = x86_64
MODULE_SRCS     = ${MODULE_SRC_DIR}/crc_test.cc
include ${MKDEFS}/post.mk
