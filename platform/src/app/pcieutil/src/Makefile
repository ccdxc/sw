#
# Copyright (c) 2018, Pensando Systems Inc.
#

PROGRAM = pcieutil
SOURCES = \
	pcieutil.cc \
	aximst.cc \
	bar.cc \
	dev.cc \
	link.cc \
	pmt.cc \
	prt.cc \
	$(NULL)

LCFLAGS = \
	-I $(ASIC_ROOT)/capri/model/cap_top \
	-I $(ASIC_ROOT)/capri/model/cap_top/csr_defines \
	-I $(TOPDIR)/src/lib/pal/include \
	-I $(TOPDIR)/src/lib/misc/include \
	-I $(TOPDIR)/src/lib/pciemgrutils/include \
	-I $(TOPDIR)/src/lib/pciemgr/include \
	$(NULL)

LCXXFLAGS = $(LCFLAGS)
LIBRARIES = \
	-lpciemgr \
	-lpciemgrutils \
	-lcfgspace \
	-lpcietlp \
	-lpcieport \
	-lintrutils \
	-lpal \
	-lmisc \
	$(NULL)

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
