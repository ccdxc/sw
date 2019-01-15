# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libociapi_impl.so
MODULE_PIPELINE = apollo
MODULE_SOLIBS   = lpmitree
include ${MKDEFS}/post.mk
