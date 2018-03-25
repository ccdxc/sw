#
# Copyright (c) 2017-2018, Pensando Systems Inc.
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
	-I $(ASIC_ROOT)/capri/model/cap_top \
	-I $(ASIC_ROOT)/capri/model/cap_top/csr_defines \
	-I ../include \
	$(NULL)

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
