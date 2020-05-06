# {C} Copyright 2018,2020 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := libintrutils.lib
MODULE_INCS     := ${MODULE_SRC_DIR}/${ASIC}
MODULE_SRCS	:= $(wildcard ${MODULE_SRC_DIR}/*.c) \
                   $(wildcard ${MODULE_SRC_DIR}/${ASIC}/*.c)
include ${MKDEFS}/post.mk
