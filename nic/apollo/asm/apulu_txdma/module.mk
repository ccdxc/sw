# {C} Copyright 2019 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET       = apulu_txdma.asmbin
MODULE_PREREQS      = apulu_txdma.p4bin
MODULE_PIPELINE     = apulu
MODULE_INCS         = ${BLD_P4GEN_DIR}/apulu_txdma/asm_out \
                      ${BLD_P4GEN_DIR}/apulu_txdma/alt_asm_out \
                      ${TOPDIR} \
                      ${TOPDIR}/nic/asm/common-p4+/include \
                      ${TOPDIR}/nic/include \
                      ${BLD_P4GEN_DIR}
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
MODULE_BIN_DIR      = ${BLD_BIN_DIR}/p4pasm_txdma
include ${MKDEFS}/post.mk
