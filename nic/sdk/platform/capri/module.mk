# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_DEFS   = ${NIC_CSR_DEFINES}
MODULE_TARGET = libsdkcapri.so
MODULE_PREREQS = hal.memrgns
include ${MKDEFS}/post.mk
