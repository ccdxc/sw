# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET       = cpu_p4plus_rxdma.asmbin
MODULE_PREREQS      = cpu_p4plus.p4bin
MODULE_PIPELINE     = iris gft
MODULE_INCS         = ${BLD_P4GEN_DIR}/cpu_rxdma/asm_out \
                      ${MODULE_DIR}/../include \
                      ${MODULE_DIR}/../../common-p4+/include \
                      ${MODULE_DIR}/../../../include
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
MODULE_SRCS         =     $(MODULE_DIR)/cpu_initial_action.s \
                          $(MODULE_DIR)/cpu_read_desc_pindex.s \
                          $(MODULE_DIR)/cpu_read_arqrx_pindex.s \
                          $(MODULE_DIR)/cpu_desc_alloc.s \
                          $(MODULE_DIR)/cpu_page_alloc.s \
                          $(MODULE_DIR)/cpu_hash_calc_id.s \
                          $(MODULE_DIR)/cpu_write_arq.s \
                          $(MODULE_DIR)/cpu_rx_ring_full_drop_error.s \
                          $(MODULE_DIR)/cpu_rx_semaphore_full_drop_error.s

MODULE_BIN_DIR      = ${BLD_BIN_DIR}/p4pasm
include ${MKDEFS}/post.mk
