#
# Copyright (c) 2017, Pensando Systems Inc.
#

PROGRAM = eth_dbgtool
SOURCES = eth_dbgtool.cc

LBUILD_ARCHES = x86_64 aarch64

LPREALL = deps

deps:
	$(MAKE) ARCH=$(BUILD_ARCH) -C $(TOPDIR)/../nic sdk
	$(MAKE) ARCH=$(BUILD_ARCH) -C $(TOPDIR)/../nic -j4 gen_proto
	$(MAKE) ARCH=$(BUILD_ARCH) -C $(TOPDIR)/../nic/gen/proto/hal -j4

LCXXFLAGS += \
	-I $(TOPDIR)/../nic/include \
	-I $(TOPDIR)/../nic/sdk/include \
	$(NULL)

LLDFLAGS = \
	-L $(TOPDIR)/../nic/conf/sdk \
	$(NULL)

SYSTEM_LIBRARIES = \
	-ldl \
	-lpthread \
	-lpal \
	-llogger \
	-lindexer \
	$(NULL)


TOPDIR = ../..
include $(TOPDIR)/make/Makefile.inc
