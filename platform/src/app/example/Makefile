#
# Copyright (c) 2018, Pensando Systems Inc.
#

PROGRAM = example
SOURCES = example.cc

LCXXFLAGS = \
	-I $(ASIC_ROOT)/capri/model/cap_top \
	-I $(ASIC_ROOT)/capri/model/cap_top/csr_defines \
	-I ../../lib/pal/include \
	$(NULL)

# Use static libraries so we don't
# have to set LD_LIBRARY_PATH to run
STATIC_LIBRARIES = \
	-lpal \
	$(NULL)

TOPDIR = ../../..
include $(TOPDIR)/make/Makefile.inc
