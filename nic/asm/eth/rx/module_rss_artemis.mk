# {C} Copyright 2019 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET       = eth_rx_rss_$(PIPELINE).asmbin
MODULE_PREREQS      = eth.p4bin
MODULE_PIPELINE     = artemis apulu athena
MODULE_INCS         = ${BLD_P4GEN_DIR}/p4plus_rxdma/asm_out \
                      ${BLD_P4GEN_DIR}/p4plus_rxdma/alt_asm_out \
                      ${MODULE_DIR}/..
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
MODULE_BIN_DIR      = ${BLD_BIN_DIR}/p4pasm_rxdma
MODULE_SRCS         = ${MODULE_SRC_DIR}/eth_rx_rss_params.s
include ${MKDEFS}/post.mk
