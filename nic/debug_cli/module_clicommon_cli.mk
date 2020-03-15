# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET       = _cli.lib
MODULE_PIPELINE	    = iris apollo artemis apulu athena
MODULE_PREREQS      = clicommon.swigcli
MODULE_SRC_DIR      = ${BLD_P4CLICMN_DIR}
MODULE_SRCS         = $(wildcard ${MODULE_SRC_DIR}/*.cc) \
                      $(wildcard ${MODULE_SRC_DIR}/src/*.cc)
MODULE_INCS		    = ${CLI_P4PD_INCS}
MODULE_FLAGS        = ${CLI_P4PD_FLAGS}
MODULE_LDLIBS       = ${CLI_${PIPELINE}_P4PD_LDLIBS}
MODULE_SOLIBS       = ${CLI_${PIPELINE}_P4PD_SOLIBS} trace
include ${MKDEFS}/post.mk
