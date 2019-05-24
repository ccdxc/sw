# {C} Copyright 2019 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET       = artemis_txdma.asmbin
MODULE_PREREQS      = artemis_txdma.p4bin
MODULE_PIPELINE     = artemis
MODULE_INCS         = ${BLD_P4GEN_DIR}/artemis_txdma/asm_out \
                      ${BLD_P4GEN_DIR}/artemis_txdma/alt_asm_out \
                      ${TOPDIR} \
                      ${TOPDIR}/nic/asm/common-p4+/include \
                      ${TOPDIR}/nic/include \
                      ${BLD_P4GEN_DIR}
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
MODULE_BIN_DIR      = ${BLD_BIN_DIR}/p4pasm_txdma
include ${MKDEFS}/post.mk
