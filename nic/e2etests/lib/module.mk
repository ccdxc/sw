# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libe2etest_lib.lib
MODULE_PIPELINE = iris gft
MODULE_ARCH     = x86_64
MODULE_SRCS     = ${MODULE_SRC_DIR}/packet.cc
include ${MKDEFS}/post.mk
