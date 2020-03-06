# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libpdstest.lib
MODULE_PIPELINE = apollo artemis apulu athena
MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/*.cc)
MODULE_SOLIBS   = sdkfru pal
include ${MKDEFS}/post.mk
