# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET       = athena_p4.asmbin
MODULE_PREREQS      = athena_p4.p4bin
MODULE_PIPELINE     = athena
MODULE_INCS         = ${BLD_P4GEN_DIR}/athena/asm_out
MODULE_INCS        += ${BLD_P4GEN_DIR}/athena/alt_asm_out
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
MODULE_BIN_DIR      = ${BLD_BIN_DIR}/p4asm
include ${MKDEFS}/post.mk
