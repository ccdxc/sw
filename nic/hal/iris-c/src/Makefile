#
# Copyright (c) 2017, Pensando Systems Inc.
#

LIBRARY = libnic.so
SOURCES = \
	nic.cc \
	hal.cc \
	vrf.cc \
	uplink.cc \
	l2seg.cc \
	lif.cc \
	enic.cc \
	endpoint.cc \
	filter.cc \
	multicast.cc \
	dev.cc \
	hntap.cc \
	$(NULL)

vpath %.cc $(TOPDIR)/../nic/e2etests/hntap/

LCFLAGS = \
	-I ../include \
	-I $(TOPDIR) \
	-I $(TOPDIR)/../ \
	-I $(TOPDIR)/../nic/hal/third-party/spdlog/include \
	-I $(TOPDIR)/../nic/e2etests/hntap \
	-I $(TOPDIR)/../nic/include/ \
	-I $(TOPDIR)/../nic/sdk/include/ \
	-I $(TOPDIR)/../nic/gen/proto/hal/ \
	-I $(TOPDIR)/../nic/hal/third-party/include/ \
	-I $(TOPDIR)/../nic/hal/third-party/grpc/include/ \
	-I $(TOPDIR)/../nic/hal/third-party/google/include/ \
	-I $(TOPDIR)/../nic/model_sim/include/ \
	-I $(TOPDIR)/../nic/asic/capri/model/cap_top \
	-I $(TOPDIR)/../nic/asic/capri/model/cap_top/csr_defines \
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
	-lpthread \
	-lgrpc \
	-lgrpc++ \
	-lgrpc_unsecure \
	-lgrpc++_unsecure \
	$(TOPDIR)/../nic/hal/third-party/google/$(BUILD_ARCH)/lib/libprotobuf.so.14 \
	-lpal \
	-lhalproto \
	-lindexer \
	-llogger \
	$(NULL)

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
