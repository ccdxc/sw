# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET = hal_access.bin
MODULE_PIPELINE = iris gft
MODULE_SOLIBS = sdkpal logger
MODULE_LDLIBS = dl
include ${MKDEFS}/post.mk
