# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   := libhal_api.so
MODULE_PIPELINE := iris
MODULE_INCS     := ${MODULE_SRC_DIR}/../include
include ${MKDEFS}/post.mk
