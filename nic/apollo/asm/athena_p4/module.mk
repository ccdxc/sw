# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk

ifeq "${P4VER}" "P4_16"
MODULE_PIPELINE   = athena_dontuse
MODULE_TARGET     = athena_p4.dontuse
else
MODULE_TARGET       = athena_p4.asmbin
MODULE_PREREQS      = athena_p4.p4bin
MODULE_PIPELINE     = athena
MODULE_INCS         = ${BLD_P4GEN_DIR}/athena/asm_out
MODULE_INCS        += ${BLD_P4GEN_DIR}/athena/alt_asm_out
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
MODULE_BIN_DIR      = ${BLD_BIN_DIR}/p4asm
endif
include ${MKDEFS}/post.mk
