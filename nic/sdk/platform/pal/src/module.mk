# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_ARCH     := aarch64
MODULE_TARGET   := libpal.so
MODULE_INCS     := ${MODULE_SRC_DIR}/../include/
MODULE_FLAGS    := -fno-tree-loop-distribute-patterns
include ${MKDEFS}/post.mk
