# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET       = gc_tx.asmbin
MODULE_PREREQS      = gc.p4bin
MODULE_PIPELINE     = iris gft
MODULE_INCS         = ${BLD_P4GEN_DIR}/gc_txdma/asm_out \
                      ${BLD_P4GEN_DIR}/gc_txdma/alt_asm_out \
                      ${MODULE_DIR}/.. \
                      ${TOPDIR}/nic/asm/common-p4+/include \
                      ${TOPDIR}/nic/asm//proxy/include \
                      ${TOPDIR}/nic/p4/include \
                      ${TOPDIR}/nic/include 
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
MODULE_BIN_DIR      = ${BLD_BIN_DIR}/p4pasm
include ${MKDEFS}/post.mk
