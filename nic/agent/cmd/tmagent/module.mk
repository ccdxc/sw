# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := tmagent.gobin
MODULE_PREREQS  := agent_irisproto.submake libdelphishm.a netproto_delphidp.submake sysmgr_delphidp.submake nmd_delphi.submake
MODULE_PIPELINE := iris
MODULE_FLAGS    := -ldflags="-s -w"
MODULE_DEPS     := $(shell find ${NICDIR}/agent/tmagent -name '*.go')
include ${MKDEFS}/post.mk
