# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := libevutils.lib
MODULE_INCS     := ${MODULE_SRC_DIR}/../include/
MODULE_LDLIBS   := ev
MODULE_FLAGS    := -DLIBEV
include ${MKDEFS}/post.mk
