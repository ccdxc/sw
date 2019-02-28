# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libhalapollosvc.so
MODULE_PIPELINE = apollo
MODULE_INCS     = ${MODULE_GEN_DIR}
include ${MKDEFS}/post.mk
