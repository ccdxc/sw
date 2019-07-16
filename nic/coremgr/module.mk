# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = coremgr.bin
MODULE_PIPELINE = iris
MODULE_PREREQS  = libsdkcoremgr.so
MODULE_SOLIBS   = sdkcoremgr logger
MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/*.cc)
include ${MKDEFS}/post.mk
