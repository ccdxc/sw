# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := netagent.gobin
MODULE_PIPELINE := iris apulu
MODULE_PREREQS  := agent_irisproto.submake agent_apuluproto.submake
MODULE_FLAGS    := -tags ${PIPELINE} -ldflags="-s -w"
MODULE_DEPS     := $(shell find ${NICDIR}/agent/dscagent -name '*.go' | egrep -v '.pb.go$$')
include ${MKDEFS}/post.mk
