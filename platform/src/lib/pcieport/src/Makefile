#
# Copyright (c) 2017, Pensando Systems Inc.
#

LIBRARY = libpcieport.a libpcieport.so
SOURCES = \
	pcieport.c \
	ctrl.c \
	intr.c \
	fsm.c \
	utils.c \
	$(NULL)

LCFLAGS += \
	-I $(ASIC_ROOT)/capri/model/cap_top \
	-I $(ASIC_ROOT)/capri/model/cap_top/csr_defines \
	-I ../../../../include/common \
	-I ../../misc/include \
	-I ../../pciehost/include \
	-I ../../pal/include \
	-I ../include \
	$(NULL)

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
