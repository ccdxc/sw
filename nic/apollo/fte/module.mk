# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libpdsfte.lib
#ifneq ($(PIPELINE),athena)
MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/*.cc) \
                  $(wildcard ${MODULE_SRC_DIR}/impl/stub/*.cc)
#else
#MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/*.cc) \
#                  $(wildcard ${MODULE_SRC_DIR}/impl/${PIPELINE}/*.cc)
#endif
MODULE_PIPELINE = athena
MODULE_INCS     = ${BLD_OUT_DIR}/pen_dpdk_submake/include ${MODULE_DIR}/${PIPELINE}
MODULE_SOLIBS   = ${NIC_FTL_LIBS} pdsapi_impl
MODULE_FLAGS    = -O3
MODULE_DEFS     = -DRTE_FORCE_INTRINSICS
MODULE_PREREQS  = pen_dpdk.submake
MODULE_LDLIBS   = dpdk numa
include ${MKDEFS}/post.mk
