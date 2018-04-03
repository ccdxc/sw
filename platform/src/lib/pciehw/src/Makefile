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
	tgt_port.c \
	itr_port.c \
	portmap.c \
	notify.c \
	indirect.c \
	req_int.c \
	event.c \
	intr.c \
	reset.c \
	$(NULL)

LCFLAGS += \
	-I $(ASIC_ROOT)/capri/model/cap_top \
	-I $(ASIC_ROOT)/capri/model/cap_top/csr_defines \
	-I ../../misc/include \
	-I ../../intrutils/include \
	-I ../../pal/include \
	-I ../../pcietlp/include \
	-I ../../cfgspace/include \
	-I ../../pcieport/include \
	-I ../../pciehost/include \
	-I ../include \
	$(NULL)

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
