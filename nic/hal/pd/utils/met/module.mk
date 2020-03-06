# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libpd_met.lib
MODULE_PIPELINE = iris gft
MODULE_PREREQS  = hal.memrgns
include ${MKDEFS}/post.mk
