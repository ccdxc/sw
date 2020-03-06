# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_ARCH     := aarch64
MODULE_TARGET   := libpal.lib
MODULE_INCS     := ${MODULE_SRC_DIR}/../include/ \
                   ${MODULE_SRC_DIR}
ifeq ($(ASIC),elba)
MODULE_SRCS     := $(wildcard ${MODULE_SRC_DIR}/elba/*.c)
else
MODULE_SRCS     := $(wildcard ${MODULE_SRC_DIR}/capri/*.c)
endif
MODULE_FLAGS    := -fno-tree-loop-distribute-patterns -O3
include ${MKDEFS}/post.mk
