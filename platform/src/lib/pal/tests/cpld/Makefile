#
# Copyright (c) 2018, Pensando Systems Inc.
#

PROGRAM = cpld 
SOURCES = cpld.c
LBUILD_ARCHES = aarch64

LCFLAGS = \
	-I ../../src \
	$(NULL)

LIBRARIES = \
	-lpal \
	$(NULL)

TOPDIR = ../../../../..
include $(TOPDIR)/make/Makefile.inc
