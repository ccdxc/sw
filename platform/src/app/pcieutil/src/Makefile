#
# Copyright (c) 2018, Pensando Systems Inc.
#

PROGRAM = pcieutil
SOURCES = \
	pcieutil.cc \
	link.cc \
	$(NULL)

LCFLAGS = \
	-I $(ASIC_ROOT)/capri/model/cap_top \
	-I $(ASIC_ROOT)/capri/model/cap_top/csr_defines \
	-I ../../../../src/lib/pal/include \
	$(NULL)

LCXXFLAGS = $(LCFLAGS)
LIBRARIES = \
	-lpal \
	$(NULL)

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
