# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MAKEDEFS}/pre.mk
MODULE_TARGET       = apollo.asmbin
MODULE_PREREQS      = apollo.p4bin
MODULE_PIPELINE     = apollo
MODULE_INCS         = ${BLD_GEN_DIR}/apollo/asm_out
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
include ${MAKEDEFS}/post.mk
