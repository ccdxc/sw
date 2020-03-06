# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = libp4pd_common_p4plus_rxdma.lib
MODULE_PIPELINE = iris gft
MODULE_PREREQS  = common_p4plus_rxdma.p4bin
MODULE_SRC_DIR  = ${BLD_P4GEN_DIR}/common_rxdma_actions/src/
MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/*p4pd.cc) \
                  $(wildcard ${MODULE_SRC_DIR}/*table_range.cc)
include ${MKDEFS}/post.mk
