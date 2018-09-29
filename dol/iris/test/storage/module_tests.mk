# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = libstorage_tests.so
MODULE_PIPELINE = iris
MODULE_FLAGS    = --std=c++14
MODULE_SRCS     = ${MODULE_SRC_DIR}/tests.cc \
                  ${MODULE_SRC_DIR}/nvme_dp.cc \
                  ${MODULE_SRC_DIR}/nvme_dp_tests.cc \
                  ${MODULE_SRC_DIR}/queues.cc \
                  ${MODULE_SRC_DIR}/dp_mem.cc \
                  ${MODULE_SRC_DIR}/xts.cc \
                  ${MODULE_SRC_DIR}/acc_ring.cc \
                  ${MODULE_SRC_DIR}/xts_tests.cc \
                  ${MODULE_SRC_DIR}/e2e_perf_tests.cc
MODULE_INCS     = ${TOPDIR}/nic/third-party/gflags/include \
                  ${TOPDIR}/nic/hal/third-party/grpc/include \
                  ${TOPDIR}/nic/hal/third-party/google/include \
                  ${TOPDIR}/platform/src/lib/nicmgr/include \
                  ${TOPDIR}/platform/include/common \
                  ${TOPDIR}/platform/src/lib/misc/include \
                  ${TOPDIR}/platform/src/lib/cfgspace/include \
                  ${TOPDIR}/platform/src/lib/pciehost/include \
                  ${TOPDIR}/platform/src/lib/pciehdevices/include \
                  ${TOPDIR}/platform/src/lib/pciehw/include \
                  ${TOPDIR}/platform/src/lib/pcieport/include \
                  ${BLD_GEN_DIR}/proto \
                  ${TOPDIR}/nic/sdk/include \
                  ${TOPDIR}/nic/include
MODULE_EXCLUDE_FLAGS = --std=c++11
include ${MKDEFS}/post.mk
