#
# Copyright (c) 2018, Pensando Systems Inc.
#

LBUILD_ARCHES = x86_64
PROGRAM = hal_api_filter_test
SOURCES = filter_test.cc

GTEST_ROOT = $(TOPDIR)/../nic/sdk/third-party/googletest-release-1.8.0/googletest
LCFLAGS = \
    -I ../../include \
    -I ../../trace \
    -I $(GTEST_ROOT)/include \
    -I $(TOPDIR)/../nic/sdk \
    -I $(TOPDIR)/../nic/sdk/include \
	-I $(TOPDIR)/../nic/build/${BUILD_ARCH}/iris/gen/proto \
    -I $(TOPDIR)/../nic/hal/third-party/spdlog/include/ \
    -I $(TOPDIR)/../nic/hal/third-party/google/include \
    -I $(TOPDIR)/../nic/hal/third-party/grpc/include \
    $(NULL)

LCXXFLAGS = $(LCFLAGS)
LLDFLAGS = \
    -L $(TOPDIR)/../nic/build/${BUILD_ARCH}/iris/lib \
    -L $(TOPDIR)/../nic/hal/third-party/google/$(BUILD_ARCH)/lib \
    -L $(TOPDIR)/gen/${BUILD_ARCH}/lib/ \
    $(NULL)

SYSTEM_LIBRARIES = \
    -lpthread \
    -lgrpc \
    -lgrpc++ \
    -lgrpc_unsecure \
    -lgrpc++_unsecure \
    -lhalproto \
    -lindexer \
    -llogger \
	$(GTEST_ROOT)/make/gtest.a \
    $(TOPDIR)/../nic/hal/third-party/google/$(BUILD_ARCH)/lib/libprotobuf.so.14 \
    -lev \
    $(NULL)
#$(GTEST_ROOT)/make/gtest.a

LIBRARIES += \
    -lhal_api \
    $(NULL)

TOPDIR = ../../../../..
include $(TOPDIR)/make/Makefile.inc
