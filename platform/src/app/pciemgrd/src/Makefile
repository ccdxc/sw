#
# Copyright (c) 2017-2018, Pensando Systems Inc.
#

PROGRAM = pciemgrd
SOURCES = \
	pciemgrd.cc \
	cli.cc \
	server.cc \
	gold.cc \
	logger.cc \
	delphic.cc \
	$(NULL)

LCXXFLAGS = \
	-I $(TOPDIR)/include/common \
	-I $(TOPDIR)/.. \
	-I $(TOPDIR)/../nic/hal/third-party/spdlog/include \
	-I $(TOPDIR)/../nic/hal/third-party/google/include \
	-I $(TOPDIR)/../nic/third-party/libev/include \
	-I $(TOPDIR)/../nic/build/${BUILD_ARCH}/iris \
	-I $(TOPDIR)/src/lib/evutils/include \
	-I $(TOPDIR)/src/lib/misc/include \
	-I $(TOPDIR)/src/lib/pal/include \
	-I $(TOPDIR)/src/lib/cfgspace/include \
	-I $(TOPDIR)/src/lib/pcieport/include \
	-I $(TOPDIR)/src/lib/pciehdevices/include \
	-I $(TOPDIR)/src/lib/pciemgr/include \
	-I $(TOPDIR)/src/lib/pciemgrutils/include \
	-I $(TOPDIR)/src/lib/pciemgr_if/include \
	$(NULL)

LIBRARIES = \
	-lpciemgr_if \
	-lpciemgr \
	-lpciemgrutils \
	-lpciehdevices \
	-lintrutils \
	-lcfgspace \
	-lpcieport \
	-lpcietlp \
	-lpal \
	-lmisc \
	-levutils \
	$(NULL)

LCXXFLAGS += \
	-I $(TOPDIR)/src/third-party/lib/edit/include \
	-I $(TOPDIR)/src/third-party/lib/ncurses/include \
	$(NULL)

LLDFLAGS = \
	-L$(TOPDIR)/src/third-party/lib/edit/$(BUILD_ARCH) \
	-L$(TOPDIR)/src/third-party/lib/ncurses/$(BUILD_ARCH) \
	-L$(TOPDIR)/../nic/build/${BUILD_ARCH}/iris/lib \
	$(NULL)

SYSTEM_LIBRARIES = \
	-L$(TOPDIR)/../nic/third-party/libev/$(BUILD_ARCH)/lib -lev \
	-lm \
	-ledit \
	-lncurses \
	-lpthread \
	-ldelphisdk \
	$(TOPDIR)/../nic/hal/third-party/google/$(BUILD_ARCH)/lib/libprotobuf.so.14 \
	-lsysmgr \
	$(NULL)

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
