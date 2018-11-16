# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := eth_dbgtool.bin
MODULE_SOLIBS   := sdkpal logger
MODULE_LDLIBS   := dl pthread thread
include ${MKDEFS}/post.mk
