#
# Copyright (c) 2017, Pensando Systems Inc.
#

LIBRARY = libpciehw.a libpciehw.so
SOURCES = \
	pciehw.c \
	cfg.c \
	bar.c \
	romsk.c \
	pmt.c \
	prt.c \
	vfstride.c \
	hdrt.c \
	notify.c \
	$(NULL)

LCFLAGS += \
	-I $(ASIC_ROOT)/capri/model/cap_top \
	-I $(ASIC_ROOT)/capri/model/cap_top/csr_defines \
	-I ../../misc/include \
	-I ../../pal/include \
	-I ../../cfgspace/include \
	-I ../../pciehost/include \
	-I ../include \
	$(NULL)

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
