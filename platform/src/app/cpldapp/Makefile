#
# Copyright (c) 2018, Pensando Systems Inc.
#

PROGRAM = cpldapp 
SOURCES = \
	main.cc

LCFLAGS = \
	-I $(TOPDIR)/../nic/include \
	-I $(TOPDIR)/../nic/sdk/include \
	-I $(TOPDIR)/../nic/sdk \
	$(NULL)

LCXXFLAGS = $(LCFLAGS)

LLDFLAGS = \
	-L $(TOPDIR)/../nic/gen/$(BUILD_ARCH)/lib/ \
	-L $(TOPDIR)/../nic/build/${BUILD_ARCH}/iris/lib \
	$(NULL)

SYSTEM_LIBRARIES += \
	-lsdkpal \
	-ldl \
	-llogger \
	-lpthread \
	-lthread \
	$(NULL)


TOPDIR = ../../..
include $(TOPDIR)/make/Makefile.inc
