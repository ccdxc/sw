#
# Copyright (c) 2017, Pensando Systems Inc.
#

PROGRAM = captrace
SOURCES = main.cc

LPREALL = deps

deps:
	$(MAKE) ARCH=$(BUILD_ARCH) -C $(TOPDIR)/../nic sdk
	$(MAKE) ARCH=$(BUILD_ARCH) -C $(TOPDIR)/../nic -j4 gen_proto
	$(MAKE) ARCH=$(BUILD_ARCH) -C $(TOPDIR)/../nic/gen/proto/hal -j4

LCFLAGS = \
	-I ../include \
	-I $(TOPDIR) \
	-I $(TOPDIR)/.. \
	-I $(TOPDIR)/include/common \
	-I $(TOPDIR)/src/lib/pal/include \
	-I $(TOPDIR)/src/lib/misc/include \
	-I $(TOPDIR)/../nic/include/ \
	-I $(TOPDIR)/../nic/sdk/include \
	-I $(TOPDIR)/../nic/gen/proto/hal/ \
	-I $(TOPDIR)/../nic/hal/third-party/include/ \
	-I $(TOPDIR)/../nic/hal/third-party/grpc/include/ \
	-I $(TOPDIR)/../nic/hal/third-party/google/include/ \
	-I $(TOPDIR)/../nic/model_sim/include/ \
	$(NULL)

LCXXFLAGS = $(LCFLAGS)

LLDFLAGS = \
	-L $(TOPDIR)/../nic/obj \
	-L $(TOPDIR)/../nic/conf/sdk \
	-L $(TOPDIR)/../nic/gen/$(BUILD_ARCH)/lib/ \
	-L $(TOPDIR)/../nic/hal/third-party/grpc/$(BUILD_ARCH)/lib \
	-L $(TOPDIR)/../nic/hal/third-party/google/$(BUILD_ARCH)/lib \
	$(NULL)

SYSTEM_LIBRARIES = \
	-ldl \
	-lpthread \
	-lgrpc \
	-lgrpc++ \
	-lgrpc_unsecure \
	-lgrpc++_unsecure \
	$(TOPDIR)/../nic/hal/third-party/google/$(BUILD_ARCH)/lib/libprotobuf.so.14 \
	-lhalproto \
	-llogger \
	$(NULL)

SYSTEM_LIBRARIES += \
	-lpal \
	$(NULL)


TOPDIR = ../../..
include $(TOPDIR)/make/Makefile.inc
