# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET       = poseidon_commontxdma_p4pd.swigcli
MODULE_PIPELINE     = poseidon
MODULE_PREREQS      = poseidon_txdma.p4bin
MODULE_SRC_DIR      = ${BLD_P4GEN_DIR}/p4plus_txdma/cli
MODULE_SRCS         = ${MODULE_SRC_DIR}/*.i
MODULE_FLAGS        = -c++ -python
MODULE_POSTGEN_MK   = module_txdma_cli.mk
include ${MKDEFS}/post.mk
