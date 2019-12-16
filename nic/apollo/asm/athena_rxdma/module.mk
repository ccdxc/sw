# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET       = athena_rxdma.asmbin
MODULE_PREREQS      = athena_rxdma.p4bin eth.p4bin
MODULE_PIPELINE     = athena
MODULE_INCS         = ${BLD_P4GEN_DIR}/p4plus_rxdma/asm_out \
                      ${BLD_P4GEN_DIR}/p4plus_rxdma/alt_asm_out \
                      ${TOPDIR} \
                      ${TOPDIR}/nic/asm/common-p4+/include \
                      ${TOPDIR}/nic/include \
                      ${BLD_P4GEN_DIR}
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
MODULE_BIN_DIR      = ${BLD_BIN_DIR}/p4pasm_rxdma
include ${MKDEFS}/post.mk
