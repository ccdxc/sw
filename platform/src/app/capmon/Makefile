#
# Copyright (c) 2018, Pensando Systems Inc.
#

PROGRAM = capmon
SOURCES = main.cc pcie.cc bwmon.cc rxdma.cc txdma.cc p4.cc crypto.cc pb.cc stage.cc

OPTS = "-g"

LCFLAGS = \
	-I ../include \
	-I $(TOPDIR)/src/lib/pal/include \
	-I $(ASIC_ROOT)/capri/model/cap_top \
	-I $(ASIC_ROOT)/capri/model/cap_top/csr_defines \
	$(NULL)

LCXXFLAGS = $(LCFLAGS)

LLDFLAGS = \
	$(NULL)

LIBRARIES += \
	-lpal \
	$(NULL)


TOPDIR = ../../..
include $(TOPDIR)/make/Makefile.inc
