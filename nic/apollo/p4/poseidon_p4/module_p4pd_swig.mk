# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET       = poseidon_p4pd.swigcli
MODULE_PIPELINE     = poseidon
MODULE_PREREQS      = poseidon_p4.p4bin
MODULE_SRC_DIR      = ${BLD_P4GEN_DIR}/p4/cli
MODULE_SRCS         = ${MODULE_SRC_DIR}/*.i
MODULE_FLAGS        = -c++ -python
MODULE_POSTGEN_MK   = module_p4pd_cli.mk
include ${MKDEFS}/post.mk
