# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libpt.lib
MODULE_PIPELINE = iris gft
MODULE_SOLIBS   = haltrace
include ${MKDEFS}/post.mk
