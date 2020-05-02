# {C} Copyright 2020 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_ARCH       := aarch64 x86_64
MODULE_TARGET     := pen_dpdk.submake
MODULE_CLEAN_DIRS := ${MODULE_DIR}/../../pen_dpdk/dpdk/build
#For simulator
ifeq ($(ARCH), x86_64)
MODULE_SOLIBS     := dpdksim sdkpal logger
MODULE_ARLIBS	  := dpdksim
endif
include ${MKDEFS}/post.mk
