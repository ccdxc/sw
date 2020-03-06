# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET = libplugin_proxy.lib
MODULE_PIPELINE = iris gft
MODULE_ARCH     = x86_64
MODULE_SOLIBS   = lkl_api
MODULE_LDLIBS   = lkl
include ${MKDEFS}/post.mk
