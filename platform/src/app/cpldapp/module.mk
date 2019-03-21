# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := cpldapp.bin
MODULE_SOLIBS   := sdkpal logger
MODULE_LDLIBS   := dl pthread
include ${MKDEFS}/post.mk
