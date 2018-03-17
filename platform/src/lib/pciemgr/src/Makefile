#
# Copyright (c) 2017-2018, Pensando Systems Inc.
#

LIBRARY = libpciemgr.a libpciemgr.so
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
	pciehw_dev.c \
	pciehdev_bridgedn.c \
	pciehdev_bridgeup.c \
	pciehsvc.c \
	$(NULL)

LCFLAGS += \
	-I $(ASIC_ROOT)/capri/design/common \
	-I $(ASIC_ROOT)/capri/model/cap_top \
	-I $(ASIC_ROOT)/capri/model/cap_top/csr_defines \
	-I $(TOPDIR)/include/common \
	-I ../../misc/include \
	-I ../../intrutils/include \
	-I ../../pal/include \
	-I ../../pcietlp/include \
	-I ../../cfgspace/include \
	-I ../../pciemgrutils/include \
	-I ../../pcieport/include \
	-I ../include \
	$(NULL)

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
