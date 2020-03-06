# {C} Copyright 2019 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := librdmamgr_iris.lib
MODULE_PIPELINE := iris
MODULE_INCS     := ${MODULE_SRC_DIR} \
				   ${TOPDIR}/nic/sdk/platform/rdmamgr \
				   ${BLD_P4GEN_DIR}/common_rxdma_actions/include  \
				   ${BLD_P4GEN_DIR}/common_txdma_actions/include
include ${MKDEFS}/post.mk
