# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libpdsapi_${PIPELINE}_impl.so
MODULE_PIPELINE = artemis
MODULE_SOLIBS   = lpmitree rfc sensor trace memhash
MODULE_DEFS     = -DCAPRI_SW ${NIC_CSR_DEFINES}
MODULE_FLAGS    = ${NIC_CSR_FLAGS}
include ${MKDEFS}/post.mk
