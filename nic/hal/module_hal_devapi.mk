# {C} Copyright 2020 Pensando Systems Inc. All rights reserved

# This is a temporary module.mk for some of the gtests
# TODO. This should be removed once we migrate devapi to functions
# instead of grpc for the below pipelines.

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libhal_devapi.so
MODULE_PIPELINE = iris gft
MODULE_SRCS     = ${MODULE_SRC_DIR}/hal_devapi.cc
include ${MKDEFS}/post.mk
