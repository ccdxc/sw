# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_ARCH     := aarch64
MODULE_TARGET   := libpal.lib
MODULE_INCS     := ${MODULE_SRC_DIR}/../include/ \
                   ${MODULE_SRC_DIR}
MODULE_SRCS     := $(wildcard ${MODULE_SRC_DIR}/*.c)
ifeq ($(ASIC),elba)
MODULE_SRCS     += $(wildcard ${MODULE_SRC_DIR}/elba/*.c)
MODULE_FLAGS    := -fno-tree-loop-distribute-patterns -O3 -DELBA
else
MODULE_SRCS     += $(wildcard ${MODULE_SRC_DIR}/capri/*.c)
MODULE_FLAGS    := -fno-tree-loop-distribute-patterns -O3
endif
include ${MKDEFS}/post.mk
