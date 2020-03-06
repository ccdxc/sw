# {C} Copyright 2019 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET = libftlite.lib
ifeq ($(PLATFORM),hw)
MODULE_FLAGS  = -O3 -DUSE_ARM64_SIMD -Wno-array-bounds
else
MODULE_FLAGS  = -O0
endif
MODULE_PIPELINE = artemis
include ${MKDEFS}/post.mk
