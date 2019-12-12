# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libpdsapi_impl.so
MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/*.cc) $(wildcard ${MODULE_SRC_DIR}/${PIPELINE}/*.cc)
MODULE_PIPELINE = apollo artemis apulu poseidon
MODULE_SOLIBS   = ${NIC_${PIPELINE}_PDSAPI_IMPL_SOLIBS}
MODULE_DEFS     = -DCAPRI_SW ${NIC_CSR_DEFINES}
MODULE_FLAGS    = ${NIC_CSR_FLAGS}
include ${MKDEFS}/post.mk
