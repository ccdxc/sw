# {C} Copyright 2019 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET = libmemhash.lib
MODULE_SOLIBS = utils indexer
MODULE_FLAGS  = -O3
include ${MKDEFS}/post.mk
