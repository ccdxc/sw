#
# Copyright (c) 2017-2018, Pensando Systems Inc.
#

LIBRARY = libpal.a libpal.so
SOURCES = \
	pal.c \
	reg.c \
	mem.c \
	mmap_region.c \
        pal_mm.c \
	trace.c \
	pal_cpld.c \
	$(NULL)

LCFLAGS += \
	-I $(ASIC_ROOT)/capri/design/common \
	-I $(ASIC_ROOT)/capri/model/cap_top \
	-I $(ASIC_ROOT)/capri/model/cap_top/csr_defines \
	-I ../include \
	-fno-tree-loop-distribute-patterns \
	$(NULL)

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
