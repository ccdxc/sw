# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = halctl_ext.submake
MODULE_PREREQS  = agent_halproto.submake
MODULE_PIPELINE = iris
MODULE_ARCH     = aarch64
include ${MKDEFS}/post.mk
