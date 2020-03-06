# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libsslutils.lib
MODULE_SOLIBS	= logger
MODULE_LDLIBS   = ssl crypto
include ${MKDEFS}/post.mk
