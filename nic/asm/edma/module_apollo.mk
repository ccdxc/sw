# {C} Copyright 2019 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET       = edma_$(PIPLINE).asmbin
MODULE_PREREQS      = edma.p4bin
MODULE_PIPELINE     = apollo artemis apulu athena
MODULE_INCS         = ${BLD_P4GEN_DIR}/edma_txdma_actions/asm_out \
                      ${BLD_P4GEN_DIR}/edma_txdma_actions/alt_asm_out \
                      ${TOPDIR}/nic/asm/common-p4+/include \
                      ${TOPDIR}/nic/p4/include \
                      ${TOPDIR}/nic/include
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
MODULE_BIN_DIR      = ${BLD_BIN_DIR}/p4pasm_txdma
include ${MKDEFS}/post.mk
