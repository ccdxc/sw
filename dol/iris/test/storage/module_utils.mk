# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_DEFS     = ${ASIC_DEFINES}
MODULE_TARGET   = libstorage_utils.lib
MODULE_PIPELINE = iris
MODULE_SRCS     = ${MODULE_SRC_DIR}/utils.cc
MODULE_INCS     = ${TOPDIR}/nic/third-party/gflags/include \
                  ${TOPDIR}/nic/hal/third-party/grpc/include \
                  ${TOPDIR}/nic/hal/third-party/google/include \
				  ${TOPDIR}/nic/sdk/include/sdk \
                  ${TOPDIR}/platform/src/lib/nicmgr/include \
                  ${TOPDIR}/platform/include/common \
                  ${TOPDIR}/platform/src/lib/pciemgr/include \
                  ${TOPDIR}/platform/src/lib/pciemgrutils/include \
                  ${TOPDIR}/platform/src/lib/pciehdevices/include \
                  ${TOPDIR}/platform/src/lib/pcieport/include \
                  ${BLD_GEN_DIR}/proto \
                  ${TOPDIR}/nic/sdk/include \
                  ${TOPDIR}/nic/include

include ${MKDEFS}/post.mk
