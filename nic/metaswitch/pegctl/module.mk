# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = pegctl.gobin
MODULE_PREREQS  = agent_pdsproto.submake pdsgen.proto ms_pdsproto.submake
MODULE_PIPELINE = apollo artemis apulu
MODULE_FLAGS    = -ldflags="-s -w"
MODULE_DEPS     = $(shell find ${MODULE_SRC_DIR}/ -name '*.go')
include ${MKDEFS}/post.mk
