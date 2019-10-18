# {C} Copyright 2019 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET       = gft16_p4.asmbin
MODULE_PIPELINE     = gft16
MODULE_INCS         = ${BLD_P4GEN_DIR}/gft16/asm_out
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
MODULE_SRC_DIR      = ${BLD_P4GEN_DIR}/gft16/asm/
MODULE_SRCS         = $(wildcard ${MODULE_SRC_DIR}/*.asm) 
MODULE_BIN_DIR      = ${BLD_BIN_DIR}/p4asm
include ${MKDEFS}/post.mk
