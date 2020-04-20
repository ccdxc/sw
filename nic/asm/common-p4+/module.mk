# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET       = common_p4plus.asmbin
MODULE_PREREQS      = common_p4plus_rxdma.p4bin common_p4plus_txdma.p4bin
MODULE_PIPELINE     = iris gft
MODULE_INCS         = ${BLD_P4GEN_DIR}/common_rxdma_actions/asm_out \
                      ${BLD_P4GEN_DIR}/common_rxdma_actions/alt_asm_out \
                      ${BLD_P4GEN_DIR}/common_txdma_actions/asm_out \
                      ${BLD_P4GEN_DIR}/common_txdma_actions/alt_asm_out
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
MODULE_BIN_DIR      = ${BLD_BIN_DIR}/p4pasm
include ${MKDEFS}/post.mk
