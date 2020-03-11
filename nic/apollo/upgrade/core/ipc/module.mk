# {C} Copyright 2020 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET        = libpdsupgipc.lib
MODULE_PIPELINE      = apulu
MODULE_INCS          = /usr/local/include \
                       ${TOPDIR}/nic/apollo/upgrade/core/ipc
MODULE_SOLIBS        = operd penipc logger utils

MODULE_SRCS          = $(wildcard ${TOPDIR}/nic/apollo/upgrade/core/ipc/*.cc)
include ${MKDEFS}/post.mk
