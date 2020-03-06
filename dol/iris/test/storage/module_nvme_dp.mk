# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = libstorage_nvme_dp.lib
MODULE_PIPELINE = iris
MODULE_ARCH     = x86_64
MODULE_SRCS     = ${MODULE_SRC_DIR}/nvme_dp.cc \
                  ${MODULE_SRC_DIR}/logger.cc \
				  ${MODULE_SRC_DIR}/dp_mem.cc
ifeq ($(ARCH), x86_64)
MODULE_SRCS     += ${MODULE_SRC_DIR}/queues.cc
endif
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

MODULE_FLAGS         = --std=c++14
MODULE_EXCLUDE_FLAGS = --std=c++11
include ${MKDEFS}/post.mk
