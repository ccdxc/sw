#
# Copyright (c) 2017, Pensando Systems Inc.
#

LIBRARY = libpciehw.a libpciehw.so
SOURCES = \
	pciehw.c \
	pciehw_shmem.c \
	$(NULL)

LCFLAGS += \
	-I ../../misc/include \
	-I ../../cfgspace/include \
	-I ../../pciehost/include \
	-I ../include \
	$(NULL)

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
