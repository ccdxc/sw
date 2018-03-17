#
# Copyright (c) 2017-2018, Pensando Systems Inc.
#

LIBRARY = libpciemgrutils.a libpciemgrutils.so
SOURCES = \
	pciehdev.c \
	pciehcfg.c \
	pciehbar.c \
	pmt.c \
	prt.c \
	pciesys_linuxuser.c \
	$(NULL)

LCFLAGS += \
	-I $(TOPDIR)/include/common \
	-I ../../misc/include \
	-I ../../cfgspace/include \
	-I ../../pciehdevices/include \
	-I ../include \
	$(NULL)

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
