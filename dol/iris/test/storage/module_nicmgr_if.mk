# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = libstorage_nicmgr_if.lib
MODULE_PIPELINE = iris
MODULE_PREREQS  = common_p4plus_rxdma.p4bin common_p4plus_txdma.p4bin hal.memrgns
MODULE_SRCS     = ${MODULE_SRC_DIR}/nicmgr_if.cc
MODULE_INCS     = ${TOPDIR}/nic/third-party/gflags/include \
                  ${TOPDIR}/nic/hal/third-party/grpc/include \
                  ${TOPDIR}/nic/hal/third-party/google/include \
				  ${TOPDIR}/nic/sdk/include/sdk \
                  ${TOPDIR}/nic//move_planner/lib \
                  ${TOPDIR}/platform/src/lib/nicmgr/include \
                  ${TOPDIR}/platform/include/common \
                  ${TOPDIR}/platform/src/lib/pciemgr/include \
                  ${TOPDIR}/platform/src/lib/pciemgrutils/include \
                  ${TOPDIR}/platform/src/lib/pciehdevices/include \
                  ${TOPDIR}/platform/src/lib/pcieport/include \
                  ${TOPDIR}/platform/src/lib/devapi_iris \
                  ${BLD_GEN_DIR}/proto \
                  ${BLD_P4GEN_DIR}/common_rxdma_actions/include/ \
                  ${BLD_P4GEN_DIR}/common_txdma_actions/include/ \
                  ${TOPDIR}/nic/sdk/include \
                  ${TOPDIR}/nic/sdk/platform/utils \
                  ${TOPDIR}/nic/include \
				  ${TOPDIR}/nic/sdk/platform/devapi
include ${MKDEFS}/post.mk
