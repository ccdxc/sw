# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = agent_irisproto.submake
MODULE_PREREQS  =
MODULE_PIPELINE = iris apulu
MODULE_DIR      := ${GOPATH}/src/github.com/pensando/sw/nic/${MODULE_DIR}
MODULE_DEPS     := $(wildcard ${NICDIR}/proto/hal/*.proto)
include ${MKDEFS}/post.mk
