# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
#
include ${MKDEFS}/pre.mk
MODULE_TARGET   = libsdklinkmgrcsr.lib
MODULE_SRCS     = ${MODULE_SRC_DIR}/pknobs.cc \
                  ${MODULE_SRC_DIR}/cap_mx_api.cc \
                  ${MODULE_SRC_DIR}/cap_bx_api.cc
MODULE_FLAGS    = ${NIC_CSR_FLAGS}
MODULE_DEFS     = -DCAPRI_SW ${NIC_CSR_DEFINES}
include ${MKDEFS}/post.mk
