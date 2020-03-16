# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   := libeth_p4plus.lib
MODULE_INCS     := ${MODULE_SRC_DIR} \
		 	${TOPDIR}/nic/sdk/platform/rdmamgr \
			${BLD_P4GEN_DIR}/common_rxdma_actions/include  \
			${BLD_P4GEN_DIR}/common_txdma_actions/include
include ${MKDEFS}/post.mk


