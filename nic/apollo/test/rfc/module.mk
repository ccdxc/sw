# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = ${PIPELINE}_rte_bitmap_test.gtest
MODULE_PIPELINE = apollo apulu
MODULE_ARCH     = x86_64
ifeq ($(ARCH),aarch64)
MODULE_INCS     = ${TOPDIR}/nic/sdk/third-party/dpdk/v18.11/lib/librte_eal/common/include \
                  ${TOPDIR}/nic/sdk/third-party/dpdk/v18.11/lib/librte_eal/common/include/config \
                  ${TOPDIR}/nic/sdk/third-party/dpdk/v18.11/lib/librte_eal/common/include/arch/arm \
                  ${TOPDIR}/nic/sdk/third-party/dpdk/v18.11/lib/librte_compat
else
MODULE_INCS     = ${TOPDIR}/nic/sdk/third-party/dpdk/v18.11/lib/librte_eal/common/include \
                  ${TOPDIR}/nic/sdk/third-party/dpdk/v18.11/lib/librte_eal/common/include/config \
                  ${TOPDIR}/nic/sdk/third-party/dpdk/v18.11/lib/librte_eal/common/include/arch/x86 \
                  ${TOPDIR}/nic/sdk/third-party/dpdk/v18.11/lib/librte_compat
endif
MODULE_FLAGS    = -DRTE_ARCH_64 -DRTE_CACHE_LINE_SIZE=64
include ${MKDEFS}/post.mk
