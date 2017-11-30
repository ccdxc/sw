#
# Copyright (c) 2017, Pensando Systems Inc.
#

PROGRAM = palreg
SOURCES = palreg.c

LCFLAGS = \
	-I ../../../misc/include \
	-I ../../../pal/include \
	$(NULL)

LIBRARIES = \
	-lpal \
	-lmisc \
	-lpciehost \
	-lcfgspace \
	-lpciehw \
	$(NULL)

TOPDIR = ../../../../..
include $(TOPDIR)/make/Makefile.inc
