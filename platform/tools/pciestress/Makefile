#
# Copyright (c) 2018, Pensando Systems Inc.
#

PROGRAM = pciestress
SOURCES = pciestress.c
LBUILD_ARCHES = x86_64

LCFLAGS = \
	-I $(TOPDIR)/src/third-party/lib/pci/include \
	$(NULL)

LLDFLAGS = /lib/x86_64-linux-gnu/libpci.so.3

SYSTEM_LIBRARIES = 

TOPDIR = ../..
include $(TOPDIR)/make/Makefile.inc
