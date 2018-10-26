# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET       = _cli.so
MODULE_PIPELINE	    = iris
MODULE_PREREQS      = clicommon.swigcli
MODULE_SRC_DIR      = ${BLD_P4CLICMN_DIR}
MODULE_SRCS         = $(wildcard ${MODULE_SRC_DIR}/*.cc) \
                      $(wildcard ${MODULE_SRC_DIR}/src/*.cc)
MODULE_INCS		    = ${CLI_P4PD_INCS}
MODULE_FLAGS        = ${CLI_P4PD_FLAGS}
MODULE_LDLIBS       = ${CLI_P4PD_LDLIBS}
MODULE_SOLIBS       = ${CLI_P4PD_SOLIBS}

include ${MKDEFS}/post.mk
