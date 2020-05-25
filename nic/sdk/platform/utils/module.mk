# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET = libsdkplatformutils.lib
MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/*.cc)
ifeq ($(ASIC),elba)
MODULE_FLAGS    = -DELBA
endif
include ${MKDEFS}/post.mk
