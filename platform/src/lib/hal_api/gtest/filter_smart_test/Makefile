#
# Copyright (c) 2018, Pensando Systems Inc.
#

LBUILD_ARCHES = x86_64
PROGRAM = hal_api_filter_smart_test
SOURCES = filter_smart_test.cc

GTEST_ROOT = $(TOPDIR)/../nic/sdk/third-party/googletest-release-1.8.0/googletest
LCFLAGS = \
    -I ../../include \
    -I ../../trace \
    -I $(GTEST_ROOT)/include \
    -I $(TOPDIR)/../nic/sdk \
    -I $(TOPDIR)/../nic/sdk/include \
	-I $(TOPDIR)/../nic/build/${BUILD_ARCH}/iris/gen/proto \
    -I $(TOPDIR)/../nic/hal/third-party/spdlog/include/ \
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

LIBRARIES += \
    -lhal_api \
    -lhal_api_trace \
    $(NULL)

TOPDIR = ../../../../..
include $(TOPDIR)/make/Makefile.inc
