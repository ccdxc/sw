# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
ifeq "${P4VER}" "P4_14"
MODULE_TARGET       = iris.asmbin
MODULE_PREREQS      = iris.p4bin
MODULE_PIPELINE     = iris
MODULE_INCS         = ${BLD_P4GEN_DIR}/p4/asm_out
MODULE_INCS        += ${BLD_P4GEN_DIR}/p4/alt_asm_out
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
MODULE_BIN_DIR      = ${BLD_BIN_DIR}/p4asm
else
MODULE_TARGET       = iris_asmbin.dontuse
MODULE_PREREQS      = iris.p4bin
MODULE_PIPELINE     = iris
MODULE_P4C          = P4_16
endif
include ${MKDEFS}/post.mk
