# {C} Copyright 2019 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET       = eth_tx_artemis.asmbin
MODULE_PREREQS      = eth.p4bin artemis_txdma.p4bin
MODULE_PIPELINE     = artemis
MODULE_INCS         = ${BLD_P4GEN_DIR}/eth_txdma_actions/asm_out \
                      ${BLD_P4GEN_DIR}/eth_txdma_actions/alt_asm_out \
                      ${MODULE_DIR}/..
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
MODULE_BIN_DIR      = ${BLD_BIN_DIR}/p4pasm_txdma
include ${MKDEFS}/post.mk
