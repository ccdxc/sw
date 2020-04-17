# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libpdsapi_asic_impl.lib
MODULE_PIPELINE = apollo artemis apulu athena
MODULE_DEFS     = -DCAPRI_SW ${NIC_CSR_DEFINES}
MODULE_FLAGS    = ${NIC_CSR_FLAGS}
include ${MKDEFS}/post.mk
