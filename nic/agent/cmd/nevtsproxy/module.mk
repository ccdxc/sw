# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := nevtsproxy.gobin
MODULE_PREREQS  := agent_irisproto.submake nmd_delphi.submake sysmgr_delphidp.submake
MODULE_PIPELINE := iris
MODULE_DEPS     := $(shell find ${NICDIR}/agent/nevtsproxy -name '*.go')
MODULE_FLAGS    := -ldflags="-s -w"
include ${MKDEFS}/post.mk
