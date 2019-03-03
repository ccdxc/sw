# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = hub_objects.submake
MODULE_PIPELINE = iris
MODULE_PREREQS = libhalproto.so libdelphisdk.so
include ${MKDEFS}/post.mk
