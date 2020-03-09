# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = liblinkmgr_src.lib
MODULE_PIPELINE = iris gft
MODULE_SRCS     = ${MODULE_SRC_DIR}/linkmgr.cc \
                  ${MODULE_SRC_DIR}/cpu_hal_if.cc \
                  ${MODULE_SRC_DIR}/utils.cc
MODULE_INCS     = ${BLD_PROTOGEN_DIR}
MODULE_FLAGS    = ${NIC_CSR_FLAGS}
MODULE_DEFS     = -DCAPRI_SW ${NIC_CSR_DEFINES}
MODULE_SOLIBS   = linkmgrdelphi
include ${MKDEFS}/post.mk
