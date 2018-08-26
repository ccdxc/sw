# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MAKEDEFS}/pre.mk
MODULE_TARGET       = apollo2_p4.asmbin
MODULE_PREREQS      = apollo2_p4.p4bin
MODULE_PIPELINE     = apollo2
MODULE_INCS         = ${BLD_GEN_DIR}/apollo2/asm_out
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
include ${MAKEDEFS}/post.mk
