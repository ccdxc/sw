#
# Copyright (c) 2018, Pensando Systems Inc.
#

PROGRAM = pciemgrs
SOURCES = pciemgrs.c
# x86_64 test tool only
LBUILD_ARCHES = x86_64

LCFLAGS = \
	-I $(TOPDIR)/../nic/third-party/libev/include \
	-I $(TOPDIR)/src/lib/evutils/include \
	-I $(TOPDIR)/src/lib/pciemgrutils/include \
	-I $(TOPDIR)/src/lib/pciemgr_if/include \
	$(NULL)

LIBRARIES = \
	-lpciemgr_if \
	-lpciemgrutils \
	-lcfgspace \
	-lmisc \
	-levutils \
	$(NULL)

SYSTEM_LIBRARIES += \
	-L $(TOPDIR)/../nic/third-party/libev/$(BUILD_ARCH)/lib -lev \
	-lm \
	$(NULL)

TOPDIR = ../../../../..
include $(TOPDIR)/make/Makefile.inc
