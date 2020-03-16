# {C} Copyright 2020 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_ARCH     := aarch64
MODULE_TARGET   := libmarvell.lib
MODULE_INCS     := ${MODULE_SRC_DIR}/../
MODULE_SOLIBS	:= pal
include ${MKDEFS}/post.mk
