#
# Copyright (c) 2018, Pensando Systems Inc.
#

PROGRAM = portcfg
SOURCES = portcfg.c libportcfg.c
LBUILD_ARCHES = x86_64

LCFLAGS += \
	-I $(ASIC_ROOT)/capri/model/cap_top \
	-I $(ASIC_ROOT)/capri/model/cap_top/csr_defines \
	-I ../../../pal/include \
	-I ../../../pcieport/include \
	$(NULL)

LIBRARIES = \
	-lpal \
	$(NULL)

TOPDIR = ../../../../..
include $(TOPDIR)/make/Makefile.inc
