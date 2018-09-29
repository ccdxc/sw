# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET       = cpu_txdma.asmbin
MODULE_PREREQS      = cpu_p4plus.p4bin
MODULE_PIPELINE     = iris gft
MODULE_INCS         = ${BLD_P4GEN_DIR}/cpu_txdma/asm_out \
                      ${MODULE_DIR}/../include \
                      ${MODULE_DIR}/../../common-p4+/include \
                      ${MODULE_DIR}/../../../include
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
MODULE_BIN_DIR      = ${BLD_BIN_DIR}/p4pasm
include ${MKDEFS}/post.mk
