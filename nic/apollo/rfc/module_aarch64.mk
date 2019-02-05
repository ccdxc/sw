# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = librfc_aarch64.so
MODULE_PIPELINE = apollo
MODULE_ARCH     = aarh64
MODULE_INCS     = ${TOPDIR}/nic/sdk/third-party/dpdk/v18.11/lib/librte_eal/common/include \
                  ${TOPDIR}/nic/sdk/third-party/dpdk/v18.11/lib/librte_eal/common/include/config \
                  ${TOPDIR}/nic/sdk/third-party/dpdk/v18.11/lib/librte_eal/common/include/arch/arm \
                  ${TOPDIR}/nic/sdk/third-party/dpdk/v18.11/lib/librte_compat
MODULE_FLAGS    = -DRTE_ARCH_64 -DRTE_CACHE_LINE_SIZE=64
include ${MKDEFS}/post.mk
