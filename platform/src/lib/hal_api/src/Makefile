#
# Copyright (c) 2018, Pensando Systems Inc.
#

LIBRARY = libhal_api.so
SOURCES = \
    endpoint.cc \
    enic.cc \
    ethlif.cc \
    filter.cc \
    hal.cc \
    hal_client.cc \
    hal_grpc_client.cc \
    l2seg.cc \
    lif.cc \
    multicast.cc \
    print.cc \
    uplink.cc \
    vrf.cc \
    $(NULL)


LCFLAGS = \
          -I $(TOPDIR)/../nic/hal/third-party/spdlog/include/ \
          -I $(TOPDIR)/../nic/build/${BUILD_ARCH}/iris/gen/proto/ \
          -I $(TOPDIR)/../nic/sdk \
          -I $(TOPDIR)/../nic/sdk/include \
          -I $(TOPDIR)/../nic/hal/third-party/google/include \
          -I $(TOPDIR)/../nic/hal/third-party/grpc/include \
          -I $(TOPDIR)/../storage/offload/include \
          -I ../include \
          -I ../../nicmgr/include \
          $(NULL)

          #-I $(TOPDIR)/../nic/hal/third-party/grpc/include/grpc++/impl/codegen
LCXXFLAGS = $(LCFLAGS)

#LIBRARIES = \
#    -lmisc \
#    -lpciehost \
#    -lpciehdevices \
#    -lpciehw \
#    -lintrutils \
#    -lcfgspace \
#    -lpcieport \
#    -lpcietlp \
#    $(NULL)
#
#LLDFLAGS = \
#    -L $(TOPDIR)/../nic/obj \
#    -L $(TOPDIR)/../nic/conf/sdk \
#    -L $(TOPDIR)/../nic/gen/$(BUILD_ARCH)/lib/ \
#    -L $(TOPDIR)/../nic/hal/third-party/grpc/$(BUILD_ARCH)/lib \
#    -L $(TOPDIR)/../nic/hal/third-party/google/$(BUILD_ARCH)/lib \
#    -L $(TOPDIR)/../nic/build/${BUILD_ARCH}/iris/lib/ \
#    $(NULL)

#SYSTEM_LIBRARIES = \
#    -lpthread \
#    -lgrpc \
#    -lgrpc++ \
#    -lgrpc_unsecure \
#    -lgrpc++_unsecure \
#    $(TOPDIR)/../nic/hal/third-party/google/$(BUILD_ARCH)/lib/libprotobuf.so.14 \
#    -lhalproto \
#    -lindexer \
#    -llogger \
#    $(NULL)

#ifeq ($(BUILD_ARCH), "x86_64")
#    SYSTEM_LIBRARIES += -lpal
#else
#    LIBRARIES += -l libpal
#endif

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
