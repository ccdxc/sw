# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MAKEDEFS}/pre.mk
MODULE_TARGET       = apollo2_txdma.asmbin
MODULE_PREREQS      = apollo2_txdma.p4bin
MODULE_PIPELINE     = apollo2
MODULE_INCS         = ${BLD_GEN_DIR}/apollo2_txdma/asm_out \
                      ${BLD_GEN_DIR}/apollo2_txdma/alt_asm_out \
                      ${TOPDIR} \
                      ${TOPDIR}/nic/asm/common-p4+/include \
                      ${TOPDIR}/nic/include
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
include ${MAKEDEFS}/post.mk
