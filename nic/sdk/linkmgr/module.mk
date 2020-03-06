# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libsdklinkmgr.lib
MODULE_SOLIBS   = sdkxcvrdriver
MODULE_DEFS     = -DNRMAKE
include ${MKDEFS}/post.mk
