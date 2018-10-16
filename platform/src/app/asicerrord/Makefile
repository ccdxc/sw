#
# Copyright (c) 2018, Pensando Systems Inc.
#

PROGRAM = asicerrord
SOURCES = \
	capri.cc \
	main.cc \
	logger.cc

LCFLAGS = \
	-I $(TOPDIR)/../nic/include \
	-I $(TOPDIR)/../nic/sdk/include \
	-I $(TOPDIR)/../nic/sdk \
	-I $(TOPDIR)/../nic/hal/third-party/spdlog/include \
	-I $(TOPDIR)/../nic/asic/capri/design/common \
	-I $(TOPDIR)/../nic/asic/capri/model/cap_top/csr_defines \
	$(NULL)

LCXXFLAGS = $(LCFLAGS)

LLDFLAGS = \
	-L $(TOPDIR)/../nic/conf/sdk \
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
