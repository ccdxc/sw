# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET       = p4pd.swigcli
MODULE_PIPELINE     = iris
MODULE_PREREQS      = iris.p4bin
MODULE_SRC_DIR      = ${BLD_P4GEN_DIR}/p4/cli
MODULE_SRCS         = ${MODULE_SRC_DIR}/*.i
MODULE_FLAGS        = -c++ -python

#MODULE_SRCS         = $(wildcard ${MODULE_SRC_DIR}/*p4pd_cli_backend.cc) \
                      $(wildcard ${MODULE_SRC_DIR}/*wrap.cc)
#MODULE_INCS         = ${BLD_P4GEN_DIR}/common_rxdma_actions/include \
#                      ${CLI_P4PD_INCS} 
#MODULE_FLAGS        = ${CLI_P4PD_FLAGS}
#MODULE_LDLIBS       = ${CLI_P4PD_LDLIBS}
#MODULE_SOLIBS       = ${CLI_P4PD_SOLIBS}
include ${MKDEFS}/post.mk
