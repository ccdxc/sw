# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_ARCH     := aarch64
MODULE_TARGET   := test_pal_msi_ev.bin
MODULE_SOLIBS   := evutils intrutils pal
include ${MKDEFS}/post.mk
