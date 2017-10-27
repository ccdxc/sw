#
# Copyright (c) 2017, Pensando Systems Inc.
#

LIBRARY = libsimlib.a libsimlib.so
SOURCES = \
	simserver.c \
	simclient.c \
	simreadwrite.c \
	simsocket.c \
	$(NULL)

LCFLAGS += \
	-I ../../../pciehsvc/include \
	-I ../include \
	$(NULL)

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
