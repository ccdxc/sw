# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := fakehal.gobin
MODULE_PIPELINE := iris
MODULE_PREREQS  := agent_irisproto.submake agent_halproto.submake
MODULE_FLAGS    := -ldflags="-s -w"
MODULE_DEPS     := $(shell find ${NICDIR}/agent/cmd/fakehal -name '*.go')
include ${MKDEFS}/post.mk
