#
# Copyright (c) 2017, Pensando Systems Inc.
#

PROGRAM = pciemgrd
SOURCES = pciemgrd.cc
LCFLAGS = \
	-I ../../../../include/common \
	-I ../../../../src/lib/misc/include \
	-I ../../../../src/lib/cfgspace/include \
	-I ../../../../src/lib/pciehost/include \
	-I ../../../../src/lib/pciehdevices/include \
	$(NULL)

LCXXFLAGS = $(LCFLAGS)
LIBRARIES = \
	-lpciehost \
	-lpciehdevices \
	-lpciehw \
	-lcfgspace \
	-lmisc \
	$(NULL)

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
