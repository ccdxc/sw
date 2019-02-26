# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libhaldelphiutils.so
MODULE_PIPELINE = iris gft
MODULE_PREREQS  = libhalproto.so
MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/utils.cc)
MODULE_SOLIBS   = delphisdk
MODULE_LDLIBS   = rt ev
include ${MKDEFS}/post.mk
