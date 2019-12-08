# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libpdstest.so
MODULE_PIPELINE = apollo artemis apulu
MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/*.cc)
include ${MKDEFS}/post.mk
