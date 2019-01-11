# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = heimdall.gobin
MODULE_PIPELINE = iris gft
MODULE_PREREQS  = agent_halproto.submake
MODULE_PIPELINE = iris
MODULE_FLAGS    = -ldflags="-s -w"
include ${MKDEFS}/post.mk
