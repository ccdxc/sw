# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET      = tins.export
MODULE_PIPELINE    = iris gft
MODULE_EXPORT_LIBS = $(wildcard ${MODULE_EXPORT_DIR}/lib/*.so*)
include ${MKDEFS}/post.mk
