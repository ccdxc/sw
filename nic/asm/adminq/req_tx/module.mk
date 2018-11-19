# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET       = adminq_req.asmbin
MODULE_PREREQS      = adminq.p4bin
MODULE_PIPELINE     = iris gft
MODULE_INCS         = ${BLD_P4GEN_DIR}/adminq_txdma_actions/asm_out \
                      ${BLD_P4GEN_DIR}/adminq_txdma_actions//alt_asm_out \
                      ${TOPDIR}/nic/asm/common-p4+/include \
                      ${TOPDIR}/nic/p4/include \
                      ${TOPDIR}/nic/include \
                      ${TOPDIR}/nic/asic/capri/model/cap_top \
                      ${TOPDIR}/nic/asic/capri/model/cap_top/csr_defines
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
MODULE_BIN_DIR      = ${BLD_BIN_DIR}/p4pasm
include ${MKDEFS}/post.mk
