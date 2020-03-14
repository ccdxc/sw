# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   := libnicmgr_iris.lib
MODULE_PIPELINE := iris
MODULE_PREREQS  := nicmgr.proto
MODULE_SOLIBS   := nicmgr mnet pciemgr_if intrutils evutils devapi_iris rdmamgr_iris misc
MODULE_INCS     := ${BLD_PROTOGEN_DIR}/ \
                   ${TOPDIR}/nic/sdk/platform/devapi \
                   ${TOPDIR}/platform/src/lib/nicmgr/include \
                   ${BLD_P4GEN_DIR}/common_rxdma_actions/include  \
                   ${BLD_P4GEN_DIR}/common_txdma_actions/include
MODULE_LDLIBS   := crypto ${NIC_COMMON_LDLIBS} \
		   ${NIC_THIRDPARTY_GOOGLE_LDLIBS}
MODULE_SRCS     = ${MODULE_SRC_DIR}/nicmgr_init.cc ${MODULE_SRC_DIR}/upgrade.cc \
				  ${MODULE_SRC_DIR}/nicmgr_ncsi.cc  ${MODULE_SRC_DIR}/upgrade_rel_a2b.cc
include ${MKDEFS}/post.mk
