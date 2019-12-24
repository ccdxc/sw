# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libpdsapi.so
MODULE_PIPELINE = apollo artemis apulu athena
MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/*.cc) $(wildcard ${MODULE_SRC_DIR}/core/*.cc)
MODULE_SOLIBS   = asicerror
include ${MKDEFS}/post.mk
