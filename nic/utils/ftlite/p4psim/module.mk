# {C} Copyright 2019 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET = libftlite_p4psim.so
ifeq ($(PLATFORM),hw)
MODULE_FLAGS  = -O3 -DUSE_ARM64_SIMD
else
MODULE_FLAGS  = -O0
endif
MODULE_PIPELINE = artemis
include ${MKDEFS}/post.mk
