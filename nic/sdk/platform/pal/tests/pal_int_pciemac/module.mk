# {C} Copyright 2019 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_ARCH     := aarch64
MODULE_TARGET   := test_pal_int_pciemac.bin
MODULE_SOLIBS   := evutils intrutils pal
MODULE_LDLIBS   := ev
include ${MKDEFS}/post.mk
