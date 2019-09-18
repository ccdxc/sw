# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET       = apulu_commonrxdma_p4pd.swigcli
MODULE_PIPELINE		= apulu
MODULE_PREREQS      = apulu_rxdma.p4bin
MODULE_SRC_DIR      = ${BLD_P4GEN_DIR}/apulu_rxdma/cli
MODULE_SRCS         = ${MODULE_SRC_DIR}/*.i
MODULE_FLAGS        = -c++ -python
MODULE_POSTGEN_MK   = module_rxdma_cli.mk
include ${MKDEFS}/post.mk
