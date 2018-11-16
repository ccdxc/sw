# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_ARCH     := x86_64
MODULE_TARGET   := test_pciemgrd_cfg.submake
MODULE_PREREQS  := pciemgrd.bin
include ${MKDEFS}/post.mk
