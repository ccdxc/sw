# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
ifeq "${P4VER}" "P4_14"
MODULE_TARGET       = _iris_libp4_p4pdcli.lib
MODULE_PIPELINE     = iris
MODULE_PREREQS      = iris_p4pd.swigcli
MODULE_SRC_DIR      = ${BLD_P4GEN_DIR}/p4
MODULE_SRCS         = ${MODULE_SRC_DIR}/src/p4pd_cli_backend.cc) \
                      ${MODULE_SRC_DIR}/src/p4plus_entry_packing.cc) \
                      $(wildcard ${MODULE_SRC_DIR}/cli/*.cc)
MODULE_INCS         = ${BLD_P4GEN_DIR}/common_rxdma_actions/cli \
                      ${CLI_P4PD_INCS} 
MODULE_FLAGS        = ${CLI_P4PD_FLAGS}
MODULE_LDLIBS       = ${CLI_iris_P4PD_LDLIBS}
MODULE_SOLIBS       = ${CLI_iris_P4PD_SOLIBS}
else
MODULE_PIPELINE     = iris_dontuse
MODULE_TARGET       = _iris_libp4_p4pdcli.lib.dontuse
endif

include ${MKDEFS}/post.mk
