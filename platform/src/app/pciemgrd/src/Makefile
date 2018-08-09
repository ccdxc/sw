#
# Copyright (c) 2017, Pensando Systems Inc.
#

PROGRAM = pciemgrd
SOURCES = pciemgrd.cc
LCFLAGS = \
	-I $(TOPDIR)/include/common \
	-I $(TOPDIR)/src/lib/misc/include \
	-I $(TOPDIR)/src/lib/pal/include \
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

LCXXFLAGS += \
	-I $(TOPDIR)/src/third-party/lib/edit/include \
	-I $(TOPDIR)/src/third-party/lib/ncurses/include \
	$(NULL)

LLDFLAGS = \
	-L$(TOPDIR)/src/third-party/lib/edit/$(BUILD_ARCH) \
	-L$(TOPDIR)/src/third-party/lib/ncurses/$(BUILD_ARCH) \
	$(NULL)

SYSTEM_LIBRARIES = \
	-ledit \
	-lncurses \
	$(NULL)

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
