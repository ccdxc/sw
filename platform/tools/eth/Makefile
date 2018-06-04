#
# Copyright (c) 2017, Pensando Systems Inc.
#

PROGRAM = eth_dbgtool

SOURCES = eth_dbgtool.cc

LBUILD_ARCHES = aarch64

# Use static libraries so we don't
# have to set LD_LIBRARY_PATH to run
STATIC_LIBRARIES = \
	-lpal \
	$(NULL)

LCXXFLAGS += \
	-I $(TOPDIR)/../nic/include \
	-I $(TOPDIR)/src/lib/pal/include \
	$(NULL)

TOPDIR = ../..
include $(TOPDIR)/make/Makefile.inc
