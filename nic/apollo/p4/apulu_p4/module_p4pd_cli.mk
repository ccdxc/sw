# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET       = _apulu_libapulu_p4pdcli.so
MODULE_PIPELINE     = apulu
MODULE_PREREQS      = apulu_p4pd.swigcli
MODULE_SRC_DIR      = ${BLD_P4GEN_DIR}/p4
MODULE_SRCS         = $(wildcard ${MODULE_SRC_DIR}/cli/*.cc) \
                      ${MODULE_SRC_DIR}/src/p4pd_cli_backend.cc \
                      ${MODULE_SRC_DIR}/src/p4plus_entry_packing.cc
MODULE_INCS         = ${BLD_P4GEN_DIR}/common_rxdma_actions/cli \
                      ${CLI_P4PD_INCS}
MODULE_FLAGS        = ${CLI_P4PD_FLAGS}
MODULE_LDLIBS       = ${CLI_${PIPELINE}_P4PD_LDLIBS}
MODULE_SOLIBS       = ${CLI_${PIPELINE}_P4PD_SOLIBS}
include ${MKDEFS}/post.mk
