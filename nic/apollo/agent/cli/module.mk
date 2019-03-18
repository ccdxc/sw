# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = pdsctl.gobin
MODULE_PREREQS  = agent_pdsproto.submake
MODULE_PIPELINE = apollo
MODULE_FLAGS    = -ldflags="-s -w"
MODULE_DEPS     = $(shell find ${MODULE_SRC_DIR}/ -name '*.go')
include ${MKDEFS}/post.mk
