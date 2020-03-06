# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := fwloggen.gobin
MODULE_PREREQS  := agent_halproto.submake libdelphishm.lib
MODULE_PIPELINE := iris
MODULE_FLAGS    := -ldflags="-s -w"
MODULE_DEPS     := $(shell find ${NICDIR}/agent/tests/fwloggen/ -name '*.go')
include ${MKDEFS}/post.mk
