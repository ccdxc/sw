# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libsdkelba.lib
MODULE_ASIC     = elba
MODULE_LDLIBS 	= ssl crypto
MODULE_FLAGS    = -O3
MODULE_DEFS     = ${NIC_CSR_DEFINES}
include ${MKDEFS}/post.mk
