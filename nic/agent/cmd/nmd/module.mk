# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   := nmd.gobin
ifeq ($(PIPELINE), iris)
MODULE_PREREQS  := agent_halproto.submake nmd_halproto.submake nmd_delphi.submake agent_delphidp.submake
else
MODULE_PREREQS  := nmd_delphi.submake sysmgr_delphidp.submake upg.submake
endif
MODULE_PIPELINE := iris apulu
MODULE_FLAGS    := -ldflags="-s -w"
MODULE_DEPS     := $(shell find ${NICDIR}/agent/nmd -name '*.go')
include ${MKDEFS}/post.mk
