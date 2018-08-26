# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MAKEDEFS}/pre.mk
MODULE_TARGET       = gft.asmbin
MODULE_PREREQS      = gft.p4bin
MODULE_PIPELINE     = gft
MODULE_INCS         = ${BLD_GEN_DIR}/gft/asm_out
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
include ${MAKEDEFS}/post.mk
