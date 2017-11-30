#
# Copyright (c) 2017, Pensando Systems Inc.
#

LIBRARY = libpal.a libpal.so
SOURCES = \
	pal.c \
	reg.c \
	mem.c \
	region.c \
	trace.c \
	$(NULL)

LCFLAGS += \
	-I ../../misc/include \
	-I ../../pciehost/include \
	-I ../include \
	$(NULL)

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
