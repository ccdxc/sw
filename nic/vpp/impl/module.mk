# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libpdsvpp_impl.so
MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/${PIPELINE}/*.cc)
MODULE_PIPELINE = apulu
#MODULE_SOLIBS   = ${NIC_${PIPELINE}_PDSAPI_IMPL_SOLIBS}
#MODULE_DEFS     = -DCAPRI_SW ${NIC_CSR_DEFINES}
#MODULE_FLAGS    = ${NIC_CSR_FLAGS}
include ${MKDEFS}/post.mk
