# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   := nmd.gobin
MODULE_PREREQS  := agent_halproto.submake nmd_halproto.submake
MODULE_PIPELINE := iris
MODULE_FLAGS    := -ldflags="-s -w"
MODULE_DEPS     := $(shell find ${NICDIR}/agent/nmd -name '*.go')
include ${MKDEFS}/post.mk
