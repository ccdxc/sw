# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = libhal_svc.so
MODULE_PIPELINE = iris gft
MODULE_PREREQS  = hal.svcgen
include ${MKDEFS}/post.mk
