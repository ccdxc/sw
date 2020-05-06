# {C} Copyright 2019 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := test_pal_int_pciemac.bin
MODULE_ARCH     := aarch64
MODULE_ASIC     := capri
MODULE_SOLIBS   := evutils intrutils pal
MODULE_LDLIBS   := ev
ifeq ($(ASIC),capri)
MODULE_DEFS     := -DASIC_CAPRI
endif
ifeq ($(ASIC),elba)
MODULE_DEFS     := -DASIC_ELBA
endif
include ${MKDEFS}/post.mk
