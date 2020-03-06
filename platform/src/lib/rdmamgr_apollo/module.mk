# {C} Copyright 2019 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := librdmamgr_apollo.lib
MODULE_PIPELINE := apollo
MODULE_INCS     := ${MODULE_SRC_DIR} \
                   ${TOPDIR}/nic/sdk/platform/rdmamgr
include ${MKDEFS}/post.mk
