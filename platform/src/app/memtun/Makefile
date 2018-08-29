#
# Copyright (c) 2018, Pensando Systems Inc.
#

PROGRAM = memtun
SOURCES = \
	pktloop.c \
	queue.c \
	tun.c \
	util.c \
	$(NULL)

ifeq ("$(BUILD_ARCH)", "aarch64")
	SOURCES += capri.c
else
	SOURCES += host.c
endif

SYSTEM_LIBRARIES = -lpthread

TOPDIR = ../../..
include $(TOPDIR)/make/Makefile.inc
