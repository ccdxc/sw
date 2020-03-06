# {C} Copyright 2020 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk

MODULE_TARGET        = libpdsupgradecore.so
MODULE_PIPELINE      = apulu
MODULE_INCS          = /usr/local/include \
                       {TOPDIR}/nic/apollo/upgrade/core/ \
                       ${BLD_PROTOGEN_DIR}/ \
                       ${TOPDIR}/nic/apollo/upgrade/include
MODULE_PREREQS       = upgrade.upgfsmgen
MODULE_LDLIBS        = stdc++ m
MODULE_FLAGS         = -O3
MODULE_EXCLUDE_FLAGS = -O2
MODULE_SRCS          = $(wildcard ${TOPDIR}/nic/apollo/upgrade/core/*.cc)
include ${MKDEFS}/post.mk
