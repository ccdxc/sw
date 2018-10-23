# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := penctl.gobin
MODULE_PIPELINE := iris
MODULE_FLAGS    := -ldflags="-s -w"
MODULE_GOPKG    := github.com/pensando/sw/penctl
include ${MKDEFS}/post.mk
