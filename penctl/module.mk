# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   := penctl.gobin
MODULE_PREREQS  = penctl.submake
MODULE_PIPELINE := iris apulu
MODULE_FLAGS    := -ldflags="-s -w"
MODULE_DEPS     = $(shell find ${MODULE_SRC_DIR}/ -name '*.go')
MODULE_DIR      := ${GOPATH}/src/github.com/pensando/sw/nic/${MODULE_DIR}
MODULE_GOPKG    := github.com/pensando/sw/penctl
include ${MKDEFS}/post.mk
