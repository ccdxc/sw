#
# Copyright (c) 2017, Pensando Systems Inc.
#

LIBRARY = libpciehsvc.a libpciehsvc.so
SOURCES = \
	pciehsvc.c \
	$(NULL)

LCFLAGS += \
	-I ../../cfgspace/include \
	-I ../../pciehost/include \
	-I ../../pciehw/include \
	-I ../include \
	$(NULL)

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
