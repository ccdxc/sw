#
# Copyright (c) 2017, Pensando Systems Inc.
#

PROGRAM = pciemgrd
SOURCES = pciemgrd.cc
LCFLAGS = \
	-I $(TOPDIR)/include/common \
	-I $(TOPDIR)/src/lib/misc/include \
	-I $(TOPDIR)/src/lib/pcieport/include \
	-I $(TOPDIR)/src/lib/cfgspace/include \
	-I $(TOPDIR)/src/lib/pciehost/include \
	-I $(TOPDIR)/src/lib/pciehw/include \
	-I $(TOPDIR)/src/lib/pciehdevices/include \
	$(NULL)

LCXXFLAGS = $(LCFLAGS)
LIBRARIES = \
	-lpciehost \
	-lpciehdevices \
	-lpciehw \
	-lintrutils \
	-lcfgspace \
	-lpcieport \
	-lpcietlp \
	-lpal \
	-lmisc \
	$(NULL)

ifeq ("$(BUILD_ARCH)", "x86_64")
SYSTEM_LIBRARIES = -lreadline
endif

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
