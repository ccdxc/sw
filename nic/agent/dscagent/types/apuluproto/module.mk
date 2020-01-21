# {C} Copyright 2020 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = agent_apuluproto.submake
MODULE_PREREQS  =
MODULE_PIPELINE = apulu iris
MODULE_DIR      := ${GOPATH}/src/github.com/pensando/sw/nic/${MODULE_DIR}
MODULE_DEPS     := $(wildcard ${NICDIR}/apollo/agent/protos/*.proto)
include ${MKDEFS}/post.mk
