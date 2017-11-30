#
# Copyright (c) 2017, Pensando Systems Inc.
#

LIBRARY = libpciehost.a libpciehost.so
SOURCES = \
	pciehdev.c \
	pciehdev_bridgedn.c \
	pciehdev_bridgeup.c \
	pciehcfg.c \
	pciehcfg_reset.c \
	pciehbar.c \
	$(NULL)

SOURCES += pciehsys_linuxuser.c

LCFLAGS += \
	-I ../../../../include/common \
	-I ../../misc/include \
	-I ../../cfgspace/include \
	-I ../../pciehw/include \
	-I ../../pciehdevices/include \
	-I ../include \
	$(NULL)

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
