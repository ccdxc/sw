# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET       = eth_rx.asmbin
MODULE_PREREQS      = eth.p4bin
MODULE_PIPELINE     = iris gft
MODULE_INCS         = ${BLD_P4GEN_DIR}/eth_rxdma_actions/asm_out \
                      ${BLD_P4GEN_DIR}/eth_rxdma_actions/alt_asm_out \
                      ${MODULE_DIR}/.. \
                      ${TOPDIR}/nic/asic/capri/model/cap_top \
                      ${TOPDIR}/nic/asic/capri/model/cap_top/csr_defines
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
MODULE_BIN_DIR      = ${BLD_BIN_DIR}/p4pasm
include ${MKDEFS}/post.mk
