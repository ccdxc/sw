#
# Copyright (c) 2017, Pensando Systems Inc.
#

PROGRAM = example
SOURCES = example.cc

LCXXFLAGS = \
	-I $(ASIC_ROOT)/capri/model/cap_top \
	-I $(ASIC_ROOT)/capri/model/cap_top/csr_defines \
	-I ../../lib/pal/include \
	$(NULL)

LIBRARIES = \
	-lpal \
	$(NULL)

TOPDIR = ../../..
include $(TOPDIR)/make/Makefile.inc
