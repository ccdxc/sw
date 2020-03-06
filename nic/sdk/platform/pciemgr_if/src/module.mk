# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := libpciemgr_if.lib
MODULE_INCS     := ${MODULE_SRC_DIR}/../include/
MODULE_SOLIBS   := evutils
MODULE_LDLIBS   := ev pthread
include ${MKDEFS}/post.mk
