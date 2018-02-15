#
# Copyright (c) 2017, Pensando Systems Inc.
#

LIBRARY = libpciehdevices.a libpciehdevices.so
SOURCES = \
	eth.c \
	nvme.c \
	accel.c \
	virtio.c \
	debug.c \
	pciestress.c \
	utils.c \
	$(NULL)

LCFLAGS = \
	-I $(ASIC_ROOT)/capri/model/cap_top \
	-I $(ASIC_ROOT)/capri/model/cap_top/csr_defines \
	-I ../../../../include/common \
	-I ../../include \
	-I ../../pciehost/include \
	-I ../include \
	$(NULL)

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
