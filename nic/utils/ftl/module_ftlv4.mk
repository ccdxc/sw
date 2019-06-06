# {C} Copyright 2019 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET = libftlv4.so
MODULE_SOLIBS = utils indexer
ifeq ($(ARCH),x86_64)
    MODULE_FLAGS  = -O0 -DFTL_ADDRESS_PREFIX=ftlv4
else
    MODULE_FLAGS  = -O3 -DFTL_ADDRESS_PREFIX=ftlv4
endif
MODULE_PIPELINE = apollo artemis
include ${MKDEFS}/post.mk
