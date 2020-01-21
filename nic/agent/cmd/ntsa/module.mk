# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := ntsa.gobin
MODULE_PREREQS  := agent_irisproto.submake libdelphishm.a
MODULE_PIPELINE := iris
MODULE_FLAGS    := -ldflags="-s -w"
MODULE_DEPS     := $(shell find ${NICDIR}/agent/cmd/ntsa -name '*.go')
include ${MKDEFS}/post.mk
