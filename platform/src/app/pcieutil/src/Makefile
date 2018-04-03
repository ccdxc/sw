#
# Copyright (c) 2018, Pensando Systems Inc.
#

PROGRAM = pcieutil
SOURCES = pcieutil.cc
LCFLAGS = \
	-I $(ASIC_ROOT)/capri/model/cap_top \
	-I $(ASIC_ROOT)/capri/model/cap_top/csr_defines \
	-I ../../../../src/lib/pal/include \
	$(NULL)

LCXXFLAGS = $(LCFLAGS)
LIBRARIES = \
	-lpal \
	$(NULL)

ifeq ("$(BUILD_ARCH)", "x86_64")
SYSTEM_LIBRARIES = -lreadline
endif

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
