# {C} Copyright 2019 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET = librte_indexer.lib
ifeq ($(ARCH),aarch64)
	MODULE_INCS = ${SDKDIR}/third-party/dpdk/v18.11/lib/librte_eal/common/include          \
				  ${SDKDIR}/third-party/dpdk/v18.11/lib/librte_eal/common/include/config   \
				  ${SDKDIR}/third-party/dpdk/v18.11/lib/librte_eal/common/include/arch/arm \
				  ${SDKDIR}/third-party/dpdk/v18.11/lib/librte_compat
else
	MODULE_INCS = ${SDKDIR}/third-party/dpdk/v18.11/lib/librte_eal/common/include          \
				  ${SDKDIR}/third-party/dpdk/v18.11/lib/librte_eal/common/include/config   \
				  ${SDKDIR}/third-party/dpdk/v18.11/lib/librte_eal/common/include/arch/x86 \
				  ${SDKDIR}/third-party/dpdk/v18.11/lib/librte_compat
endif
MODULE_FLAGS  = -O3 -DRTE_CACHE_LINE_SIZE=64 -DRTE_ARCH_64
MODULE_SOLIBS = utils
include ${MKDEFS}/post.mk
