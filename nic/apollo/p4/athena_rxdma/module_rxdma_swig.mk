# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET       = athena_commonrxdma_p4pd.swigcli
MODULE_PIPELINE     = athena
MODULE_PREREQS      = athena_rxdma.p4bin
MODULE_SRC_DIR      = ${BLD_P4GEN_DIR}/p4plus_rxdma/cli
MODULE_SRCS         = ${MODULE_SRC_DIR}/*.i
MODULE_FLAGS        = -c++ -python
MODULE_POSTGEN_MK   = module_rxdma_cli.mk
include ${MKDEFS}/post.mk
