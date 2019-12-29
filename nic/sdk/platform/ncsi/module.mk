# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_PIPELINE := iris
MODULE_ARCH     := aarch64
MODULE_TARGET   = libncsi.so
MODULE_FLAGS    = ${NIC_CSR_FLAGS} -O2
include ${MKDEFS}/post.mk
