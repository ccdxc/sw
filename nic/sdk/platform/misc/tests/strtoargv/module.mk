# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_ARCH     := x86_64
MODULE_TARGET   := test_strtoargv.bin
MODULE_PIPELINE = iris gft
MODULE_SOLIBS   := misc
include ${MKDEFS}/post.mk
