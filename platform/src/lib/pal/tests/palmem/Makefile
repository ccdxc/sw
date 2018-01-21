#
# Copyright (c) 2017, Pensando Systems Inc.
#

PROGRAM = palmem
SOURCES = palmem.c

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
	-lintrutils \
	$(NULL)

TOPDIR = ../../../../..
include $(TOPDIR)/make/Makefile.inc
