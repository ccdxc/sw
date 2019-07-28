# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET       = phoebus_p4.asmbin
MODULE_PIPELINE     = phoebus
MODULE_INCS         = ${BLD_P4GEN_DIR}/phoebus/asm_out
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
MODULE_SRC_DIR      = ${BLD_P4GEN_DIR}/phoebus/asm/
MODULE_SRCS         = $(wildcard ${MODULE_SRC_DIR}/*.asm) 
MODULE_BIN_DIR      = ${BLD_BIN_DIR}/p4asm
include ${MKDEFS}/post.mk
