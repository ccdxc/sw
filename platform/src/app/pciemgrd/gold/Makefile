#
# Copyright (c) 2018, Pensando Systems Inc.
#

PROGRAM = pciemgrd-gold
SOURCES = \
	pciemgrd.cc \
	gold.cc \
	logger.cc \
	$(NULL)
VPATH = ../src

LCXXFLAGS += -DPCIEMGRD_GOLD

LCXXFLAGS += \
	-I $(TOPDIR)/include/common \
	-I $(TOPDIR)/../nic/hal/third-party/spdlog/include \
	-I $(TOPDIR)/src/lib/pal/include \
	-I $(TOPDIR)/src/lib/pcieport/include \
	-I $(TOPDIR)/src/lib/pciehdevices/include \
	-I $(TOPDIR)/src/lib/pciemgr/include \
	-I $(TOPDIR)/src/lib/pciemgrutils/include \
	$(NULL)

LIBRARIES = \
	-lpciemgr \
	-lpciemgrutils \
	-lpciehdevices \
	-lintrutils \
	-lcfgspace \
	-lpcieport \
	-lpcietlp \
	-lpal \
	-lmisc \
	$(NULL)

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
