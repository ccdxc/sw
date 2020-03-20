# {C} Copyright 2020 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET        = libpdsupgradecore.lib
MODULE_PIPELINE      = apulu
MODULE_INCS          = /usr/local/include \
                       {TOPDIR}/nic/apollo/upgrade/core/ \
                       ${BLD_PROTOGEN_DIR}/ \
                       ${TOPDIR}/nic/apollo/upgrade/include
MODULE_PREREQS       = upgrade.upgfsmgen

MODULE_SOLIBS        = evutils pal pdsupgipc
MODULE_LDLIBS        = stdc++ m ev
#MODULE_FLAGS         = -O3 -Wunused-variable
#MODULE_EXCLUDE_FLAGS = -O2
MODULE_SRCS          = $(wildcard ${TOPDIR}/nic/apollo/upgrade/core/*.cc)
include ${MKDEFS}/post.mk
