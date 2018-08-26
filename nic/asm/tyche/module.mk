# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MAKEDEFS}/pre.mk
MODULE_TARGET       = tyche.asmbin
MODULE_PREREQS      = tyche.p4bin
MODULE_PIPELINE     = tyche
MODULE_INCS         = ${BLD_GEN_DIR}/tyche/asm_out
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
include ${MAKEDEFS}/post.mk
