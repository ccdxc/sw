# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libociapi_impl.so
MODULE_PIPELINE = apollo
MODULE_SOLIBS   = lpmitree
MODULE_DEFS     = -DCAPRI_SW ${NIC_CSR_DEFINES}
MODULE_FLAGS    = ${NIC_CSR_FLAGS}
include ${MKDEFS}/post.mk
