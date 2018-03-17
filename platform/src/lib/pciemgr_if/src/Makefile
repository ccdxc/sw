#
# Copyright (c) 2018, Pensando Systems Inc.
#

LIBRARY = libpciemgr_if.so
SOURCES = \
	pciemgr.cc \
	pciemgrs_sock.c \
	pciemgrc_sock.c \
	pm.c \
	pmsocket.c \
	pmreadwrite.c \
	$(NULL)

INCDIRS += \
	-I $(TOPDIR)/../nic/third-party/libev/include \
	-I ../../misc/include \
	-I ../../evutils/include \
	-I ../../cfgspace/include \
	-I ../../pciemgrutils/include \
	-I ../../pciemgr/include \
	-I ../include \
	$(NULL)
LCXXFLAGS = $(INCDIRS)
LCFLAGS   = $(INCDIRS)

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
