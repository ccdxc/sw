# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET = libindexer.lib
MODULE_FLAGS  = -O3
MODULE_SOLIBS = utils
include ${MKDEFS}/post.mk
