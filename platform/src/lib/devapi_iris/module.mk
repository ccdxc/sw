# {C} Copyright 2019 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := libdevapi_iris.lib
MODULE_PIPELINE := iris
MODULE_INCS     := ${MODULE_SRC_DIR} \
				   ${TOPDIR}/nic/sdk/platform/devapi
include ${MKDEFS}/post.mk
