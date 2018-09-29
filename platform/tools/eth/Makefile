#
# Copyright (c) 2017, Pensando Systems Inc.
#

PROGRAM = eth_dbgtool
SOURCES = eth_dbgtool.cc

LBUILD_ARCHES = x86_64 aarch64

LPREALL = deps

deps:
	$(MAKE) ARCH=$(BUILD_ARCH) -C $(TOPDIR)/../nic

LCXXFLAGS += \
	-I $(TOPDIR)/../nic/include \
	-I $(TOPDIR)/../nic/sdk/include \
	-I $(TOPDIR)/../nic/sdk \
	$(NULL)

LLDFLAGS = \
	-L $(TOPDIR)/../nic/conf/sdk \
	-L $(TOPDIR)/../nic/build/${BUILD_ARCH}/iris/lib \
	$(NULL)

SYSTEM_LIBRARIES = \
	-ldl \
	-lpthread \
	-lsdkpal \
	-llogger \
	-lindexer \
	$(NULL)


TOPDIR = ../..
include $(TOPDIR)/make/Makefile.inc
