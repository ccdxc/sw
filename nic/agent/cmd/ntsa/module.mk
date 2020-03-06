# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := ntsa.gobin
MODULE_PREREQS  := agent_irisproto.submake libdelphishm.lib sysmgr_delphidp.submake nmd_delphi.submake
MODULE_PIPELINE := iris
MODULE_FLAGS    := -ldflags="-s -w"
MODULE_DEPS     := $(shell find ${NICDIR}/agent/cmd/ntsa -name '*.go')
include ${MKDEFS}/post.mk
