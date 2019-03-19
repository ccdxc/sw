# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := metiterator.gobin
MODULE_PREREQS  := libdelphishm.a
MODULE_PIPELINE := iris
MODULE_ARCH     := x86_64 
MODULE_FLAGS    := -ldflags="-s -w"
MODULE_DEPS     := $(shell find ${NICDIR}/delphi/shm/test/iterate -name '*.go')
include ${MKDEFS}/post.mk
