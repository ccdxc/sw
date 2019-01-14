# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   := captrace.bin
MODULE_SOLIBS   := sdkpal logger sdkcapri_csrint sdkcapri_asicrw_if
MODULE_LDLIBS   := dl sknobs Judy
MODULE_INCS     = ${NIC_CSR_INCS}
MODULE_FLAGS    = ${NIC_CSR_FLAGS}
MODULE_DEFS     = -DCAPRI_SW ${NIC_CSR_DEFINES}
MODULE_PREREQS  := hal.memrgns
include ${MKDEFS}/post.mk
