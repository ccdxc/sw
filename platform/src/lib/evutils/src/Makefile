#
# Copyright (c) 2018, Pensando Systems Inc.
#

LIBRARY = libevutils.a libevutils.so
SOURCES = \
	fd.c \
	timer.c \
	run.c \
	stop.c \
	$(NULL)

# libev has some code that trips alias check, so -Wno-strict-aliasing
LCFLAGS += \
	-Wno-strict-aliasing \
	-I $(TOPDIR)/../nic/third-party/libev/include \
	-I ../include \
	$(NULL)

# Actually use libev now that x86_64 and aarch64 versions are available
LCFLAGS += -DLIBEV

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
