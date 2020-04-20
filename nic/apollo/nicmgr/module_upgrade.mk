# {C} Copyright 2020 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libpdsupg_nicmgr.lib
MODULE_PIPELINE = apollo artemis apulu
MODULE_INCS     := ${MODULE_SRC_DIR} \
                   ${TOPDIR}/platform/src/lib/nicmgr/include \
                   ${BLD_PROTOGEN_DIR}
MODULE_SRCS     := ${MODULE_SRC_DIR}/nicmgr_upgrade_graceful.cc \
                   ${MODULE_SRC_DIR}/nicmgr_upgrade_hitless.cc
MODULE_SOLIBS   := pdsproto
MODULE_LDLIBS   := ${NIC_THIRDPARTY_GOOGLE_LDLIBS}
include ${MKDEFS}/post.mk
