# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := eth_dbgtool.bin
MODULE_SOLIBS   := sdkpal logger sdkplatformutils shmmgr bm_allocator
MODULE_LDLIBS   := dl pthread rt
MODULE_PREREQS  := hal.memrgns
include ${MKDEFS}/post.mk
