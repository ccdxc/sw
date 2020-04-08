# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libpdsnicmgr.lib
MODULE_PIPELINE = apollo artemis apulu athena
MODULE_INCS     := ${MODULE_SRC_DIR} \
                   ${TOPDIR}/nic/sdk/platform/devapi \
                   ${TOPDIR}/platform/src/lib/nicmgr/include
MODULE_SRCS     := ${MODULE_SRC_DIR}/nicmgr.cc
MODULE_PREREQS  = hal.memrgns
include ${MKDEFS}/post.mk
