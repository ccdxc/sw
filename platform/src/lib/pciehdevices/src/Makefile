#
# Copyright (c) 2017, Pensando Systems Inc.
#

LIBRARY = libpciehdevices.a libpciehdevices.so
SOURCES = \
	enet.c \
	nvme.c \
	debug.c \
	$(NULL)

LCFLAGS = \
	-I ../../../../include/common \
	-I ../../include \
	-I ../../pciehost/include \
	-I ../include \
	$(NULL)

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
