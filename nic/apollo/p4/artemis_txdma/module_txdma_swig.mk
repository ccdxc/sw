# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET       = artemis_commontxdma_p4pd.swigcli
MODULE_PIPELINE     = artemis
MODULE_PREREQS      = artemis_txdma.p4bin
MODULE_SRC_DIR      = ${BLD_P4GEN_DIR}/artemis_txdma/cli
MODULE_SRCS         = ${MODULE_SRC_DIR}/*.i
MODULE_FLAGS        = -c++ -python
MODULE_POSTGEN_MK   = module_txdma_cli.mk
include ${MKDEFS}/post.mk
