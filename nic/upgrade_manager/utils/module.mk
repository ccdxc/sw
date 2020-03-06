# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libupgradeutils.lib
MODULE_SOLIBS   = utils trace logger
MODULE_PIPELINE = iris gft
MODULE_PREREQS  = libev.export
MODULE_LDLIBS   = z
include ${MKDEFS}/post.mk
