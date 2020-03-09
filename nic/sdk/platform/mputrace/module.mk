# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   := captrace.bin
MODULE_SOLIBS   := sdkpal logger sdkcapri_asicrw_if
MODULE_LDLIBS   := dl sknobs Judy ${SDK_THIRDPARTY_CAPRI_LDLIBS}
MODULE_FLAGS    = ${NIC_CSR_FLAGS}
MODULE_DEFS     = -DCAPRI_SW ${NIC_CSR_DEFINES}
include ${MKDEFS}/post.mk
