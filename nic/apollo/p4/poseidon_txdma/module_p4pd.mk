# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libp4pd_poseidon_txdma.so
MODULE_PREREQS  = poseidon_txdma.p4bin
MODULE_PIPELINE = poseidon
MODULE_SRC_DIR  = ${BLD_P4GEN_DIR}/p4plus_txdma/src
MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/*p4pd.cc) \
                  $(wildcard ${MODULE_SRC_DIR}/*table_range.cc) \
                  $(wildcard ${MODULE_SRC_DIR}/*entry_packing.cc)

include ${MKDEFS}/post.mk
