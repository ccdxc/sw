# {C} Copyright 2019 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET       = artemis_p4.asmbin
MODULE_PREREQS      = artemis_p4.p4bin
MODULE_PIPELINE     = artemis
MODULE_INCS         = ${BLD_P4GEN_DIR}/artemis/asm_out
MODULE_INCS        += ${BLD_P4GEN_DIR}/artemis/alt_asm_out
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
MODULE_BIN_DIR      = ${BLD_BIN_DIR}/p4asm
include ${MKDEFS}/post.mk
