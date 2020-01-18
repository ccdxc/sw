# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := tmagent.gobin
MODULE_PREREQS  := agent_halproto.submake libdelphishm.a
MODULE_PIPELINE := iris
MODULE_FLAGS    := -ldflags="-s -w"
MODULE_DEPS     := $(shell find ${NICDIR}/agent/tmagent -name '*.go')
include ${MKDEFS}/post.mk
