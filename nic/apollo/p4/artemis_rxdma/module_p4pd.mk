# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libp4pd_artemis_rxdma.lib
MODULE_PREREQS  = artemis_rxdma.p4bin
MODULE_PIPELINE = artemis
MODULE_SRC_DIR  = ${BLD_P4GEN_DIR}/p4plus_rxdma/src
MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/*p4pd.cc) \
                  $(wildcard ${MODULE_SRC_DIR}/*table_range.cc) \
                  $(wildcard ${MODULE_SRC_DIR}/*entry_packing.cc)


include ${MKDEFS}/post.mk
