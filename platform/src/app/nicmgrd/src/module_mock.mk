# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   := libnicmgr_mock.so
MODULE_PIPELINE := gft
MODULE_SRCS     := ${MODULE_SRC_DIR}/nicmgr_init_mock.cc
include ${MKDEFS}/post.mk
