# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET       = apollo_rxdma.asmbin
MODULE_PREREQS      = apollo_rxdma.p4bin
MODULE_PIPELINE     = apollo
MODULE_INCS         = ${BLD_P4GEN_DIR}/apollo_rxdma/asm_out \
                      ${BLD_P4GEN_DIR}/apollo_rxdma/alt_asm_out \
                      ${TOPDIR} \
                      ${TOPDIR}/nic/asm/common-p4+/include \
                      ${TOPDIR}/nic/include
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
MODULE_BIN_DIR      = ${BLD_BIN_DIR}/p4pasm_rxdma
include ${MKDEFS}/post.mk
