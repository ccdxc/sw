#
# Copyright (c) 2017, Pensando Systems Inc.
#

PROGRAM = pciemgrd
SOURCES = pciemgrd.cc
LCFLAGS = \
	-I ../../../../include/common \
	-I ../../../../src/lib/misc/include \
	-I ../../../../src/lib/pcieport/include \
	-I ../../../../src/lib/cfgspace/include \
	-I ../../../../src/lib/pciehost/include \
	-I ../../../../src/lib/pciehw/include \
	-I ../../../../src/lib/pciehdevices/include \
	$(NULL)

LCXXFLAGS = $(LCFLAGS)
LIBRARIES = \
	-lpciehost \
	-lpciehdevices \
	-lpciehw \
	-lintrutils \
	-lcfgspace \
	-lpcieport \
	-lpal \
	-lmisc \
	$(NULL)

ifeq ("$(BUILD_ARCH)", "x86_64")
SYSTEM_LIBRARIES = -lreadline
endif

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
