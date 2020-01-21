# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = nmd_halproto.submake
MODULE_PREREQS  = agent_halproto.submake
MODULE_PIPELINE = iris gft
MODULE_DIR      := ${GOPATH}/src/github.com/pensando/sw/nic/${MODULE_DIR}
MODULE_DEPS     := $(wildcard ${NICDIR}/proto/device.proto)
include ${MKDEFS}/post.mk
