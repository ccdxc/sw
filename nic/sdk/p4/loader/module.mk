# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET = libsdkp4loader.lib
ifeq ($(ASIC),elba)
MODULE_SRCS     := ${MODULE_SRC_DIR}/loader_elba.cc
else
MODULE_SRCS     := ${MODULE_SRC_DIR}/loader.cc
endif
include ${MKDEFS}/post.mk
