#
# Copyright (c) 2017, Pensando Systems Inc.
#

PROGRAM = simclient
SOURCES = simclient.c

LCFLAGS = \
	-I ../../../lib/misc/include \
	-I ../../../lib/pciehsvc/include \
	-I ../../libsimlib/include \
	$(NULL)

LIBRARIES = \
	-lpciehsvc \
	-lpciehw \
	-lpcietlp \
	-lintrutils \
	-lpal \
	-lpciehost \
	-lcfgspace \
	-lmisc \
	-lsimlib \
	$(NULL)

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
