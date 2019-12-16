# {C} Copyright 2019 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET       = eth_rx_$(PIPELINE).asmbin
MODULE_PREREQS      = eth.p4bin
MODULE_PIPELINE     = apollo artemis apulu athena
MODULE_INCS         = ${BLD_P4GEN_DIR}/eth_rxdma_actions/asm_out \
                      ${BLD_P4GEN_DIR}/eth_rxdma_actions/alt_asm_out \
                      ${MODULE_DIR}/..
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
MODULE_BIN_DIR      = ${BLD_BIN_DIR}/p4pasm_rxdma
ifneq ($(PIPELINE),apollo)
ALL_SRCS            := $(wildcard ${MODULE_SRC_DIR}/*.s)
RSS_SRCS            := ${MODULE_SRC_DIR}/eth_rx_rss_params.s
MODULE_SRCS         := $(filter-out $(RSS_SRCS), $(ALL_SRCS))
endif
include ${MKDEFS}/post.mk
