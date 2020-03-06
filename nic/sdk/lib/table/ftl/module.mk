# {C} Copyright 2019 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET = libsdkftl.lib
MODULE_SOLIBS = utils indexer
ifeq ($(ARCH),x86_64)
    MODULE_FLAGS  = -O0 -DFTL_ADDRESS_PREFIX=ftl
else ifeq ($(ARCH),aarch64)
    MODULE_FLAGS  = -O3 -DFTL_ADDRESS_PREFIX=ftl -DUSE_ARM64_SIMD
else
    MODULE_FLAGS  = -O3 -DFTL_ADDRESS_PREFIX=ftl
endif
MODULE_PIPELINE = apollo artemis apulu iris athena
MODULE_SRCS = $(wildcard ${MODULE_SRC_DIR}/*.cc) \
              $(wildcard ${BLD_P4GEN_DIR}/p4/src/ftl.cc)
include ${MKDEFS}/post.mk
