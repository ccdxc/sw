# {C} Copyright 2020 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = operdproto.submake
MODULE_PIPELINE = apollo artemis apulu
MODULE_DIR      := ${GOPATH}/src/github.com/pensando/sw/nic/${MODULE_DIR}
MODULE_DEPS     := $(wildcard ${MODULE_DIR}/*.proto)
include ${MKDEFS}/post.mk
