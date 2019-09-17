# {C} Copyright 2019 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET       = apulu_p4.asmbin
MODULE_PREREQS      = apulu_p4.p4bin
MODULE_PIPELINE     = apulu
MODULE_INCS			= ${MODULE_DIR}/../include
MODULE_INCS        += ${BLD_P4GEN_DIR}/apulu/asm_out
MODULE_INCS        += ${BLD_P4GEN_DIR}/apulu/alt_asm_out
MODULE_INCS        += ${TOP_DIR}
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
MODULE_BIN_DIR      = ${BLD_BIN_DIR}/p4asm
include ${MKDEFS}/post.mk
