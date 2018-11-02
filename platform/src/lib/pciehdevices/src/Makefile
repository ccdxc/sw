#
# Copyright (c) 2017-2018, Pensando Systems Inc.
#

LIBRARY = libpciehdevices.a libpciehdevices.so
SOURCES = \
	bars.c \
	eth.c \
	mgmteth.c \
	nvme.c \
	accel.c \
	virtio.c \
	debug.c \
	pciestress.c \
	rcdev.c \
	$(NULL)

LCFLAGS = \
	-I $(TOPDIR)/../nic/include \
	-I $(TOPDIR)/include/common \
	-I ../../misc/include \
	-I ../../intrutils/include \
	-I ../../pciemgrutils/include \
	-I ../include \
	$(NULL)

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
