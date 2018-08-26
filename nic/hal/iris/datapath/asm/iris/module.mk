# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MAKEDEFS}/pre.mk
MODULE_TARGET       = iris.asmbin
MODULE_PREREQS      = iris.p4bin
MODULE_PIPELINE     = iris
MODULE_INCS         = ${BLD_GEN_DIR}/p4/asm_out
MODULE_DEPS         = $(shell find ${MODULE_DIR}/ -name '*.h')
include ${MAKEDEFS}/post.mk
