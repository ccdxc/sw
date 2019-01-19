# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = halctl_ext.submake
MODULE_PREREQS  = agent_halproto.submake
MODULE_PIPELINE = iris
include ${MKDEFS}/post.mk
