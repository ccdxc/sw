#
# Copyright (c) 2018, Pensando Systems Inc.
#

LIBRARY = libintrutils.a libintrutils.so
SOURCES = \
	intrutils.c \
	$(NULL)

LCFLAGS += \
	-I $(ASIC_ROOT)/capri/model/cap_top \
	-I $(ASIC_ROOT)/capri/model/cap_top/csr_defines \
	-I ../../pal/include \
	-I ../include \
	$(NULL)

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
