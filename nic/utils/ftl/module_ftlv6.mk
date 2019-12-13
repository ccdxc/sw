# {C} Copyright 2019 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET = libftlv6.so
MODULE_SOLIBS = utils indexer
ifeq ($(ARCH),x86_64)
    MODULE_FLAGS  = -O0 -DFTL_ADDRESS_PREFIX=ftlv6
else ifeq ($(ARCH),aarch64)
    MODULE_FLAGS  = -O3 -DFTL_ADDRESS_PREFIX=ftlv6 -DUSE_ARM64_SIMD
else
    MODULE_FLAGS  = -O3 -DFTL_ADDRESS_PREFIX=ftlv6
endif
MODULE_PIPELINE = apollo artemis apulu iris athena
include ${MKDEFS}/post.mk
