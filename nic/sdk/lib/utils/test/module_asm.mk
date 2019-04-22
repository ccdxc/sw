# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = crc_test_asm
MODULE_PIPELINE = iris gft
MODULE_LDLIBS   = pthread
MODULE_ARCH     = aarch64
MODULE_SRCS     = ${MODULE_SRC_DIR}/crc_test_asm.cc
include ${MKDEFS}/post.mk
