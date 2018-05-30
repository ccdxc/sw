
# Copyright (c) 2018, Pensando Systems Inc.

TOPDIR = ../../../..

LBUILD_ARCHES = x86_64 aarch64

PROGRAM = capview

SOURCES = \
	cli.cc \
	cmds.cc \
	compl.cc \
	cvdb.cc \
	dev.cc \
	editbuf.cc \
	log.cc \
	main.cc \
	parse.cc \
	regio.cc \
	$(NULL)

LCXXFLAGS = \
	-I ../include \
	-I $(TOPDIR)/src/third-party/lib/gmp/include \
	-I $(TOPDIR)/src/third-party/lib/edit/include \
	-I $(TOPDIR)/src/third-party/lib/ncurses/include \
	$(NULL)

LLDFLAGS = \
	-L$(TOPDIR)/src/third-party/lib/gmp/$(BUILD_ARCH) \
	-L$(TOPDIR)/src/third-party/lib/edit/$(BUILD_ARCH) \
	-L$(TOPDIR)/src/third-party/lib/ncurses/$(BUILD_ARCH) \
	$(NULL)

SYSTEM_LIBRARIES = -lgmp -ledit -lncurses

include $(TOPDIR)/make/Makefile.inc
