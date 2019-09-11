# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libutils.so
MODULE_SOLIBS	= logger
MODULE_LDLIBS   = ssl crypto
include ${MKDEFS}/post.mk
