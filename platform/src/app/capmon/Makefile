#
# Copyright (c) 2018, Pensando Systems Inc.
#

PROGRAM = capmon
SOURCES = capmon.cc

# x86_64 build requires -lmodel_client
LBUILD_ARCHES = aarch64

LCXXFLAGS = \
	-I $(ASIC_ROOT)/capri/model/cap_top \
	-I $(ASIC_ROOT)/capri/model/cap_top/csr_defines \
	$(NULL)

ifeq ("$(BUILD_ARCH)", "aarch64")
# Use static libraries so we don't
# have to set LD_LIBRARY_PATH to run
STATIC_LIBRARIES = \
	-lpal \
	$(NULL)

LCXXFLAGS += \
	-I ../../lib/pal/include \
	$(NULL)
else
SYSTEM_LIBRARIES = \
	-lmodel_client \
	$(NULL)

LCXXFLAGS += \
	-I $(TOPDIR)/../nic/model_sim/include/ \
	$(NULL)

LLDFLAGS += \
	-L $(TOPDIR)/../bazel-bin/nic/model_sim/ \
	$(NULL)
endif

TOPDIR = ../../..
include $(TOPDIR)/make/Makefile.inc
