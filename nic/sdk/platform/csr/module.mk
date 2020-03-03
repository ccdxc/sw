# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
# TODO rename this library to sdk_asicrw_if
include ${MKDEFS}/pre.mk
MODULE_TARGET   = libsdkcapri_asicrw_if.so
MODULE_DEFS     = ${NIC_CSR_DEFINES}
MODULE_INCS     = ${NIC_CSR_INCS}
MODULE_SRCS     = ${MODULE_SRC_DIR}/asicrw_if.cc
include ${MKDEFS}/post.mk
