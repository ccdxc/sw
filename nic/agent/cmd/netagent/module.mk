# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := netagent.gobin
MODULE_PREREQS  := agent_halproto.submake
MODULE_PIPELINE := iris
MODULE_FLAGS    := -ldflags="-s -w"
MODULE_DEPS     := $(wildcard ${MODULE_SRC_DIR}/*.go) \
                   $(wildcard ${NICDIR}/agent/netagent/datapath/halproto/*.go)
include ${MKDEFS}/post.mk
