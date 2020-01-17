# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := netagent.gobin
MODULE_PIPELINE := iris apulu
ifeq ($(PIPELINE),iris)
MODULE_PREREQS  := agent_irisproto.submake
else
MODULE_PREREQS  := agent_apuluproto.submake
endif
MODULE_FLAGS    := -tags ${PIPELINE} -ldflags="-s -w"
MODULE_DEPS     := $(shell find ${NICDIR}/agent/ -name '*.go')
include ${MKDEFS}/post.mk
