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
	-lpciemgr \
	-lpciemgrutils \
	-lpcieport \
	-lpcietlp \
	-lintrutils \
	-lpal \
	-lcfgspace \
	-lmisc \
	-lsimlib \
	$(NULL)

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
