#
# Copyright (c) 2018, Pensando Systems Inc.
#
LIBRARY = libhal_api_trace.so
SOURCES = \
    trace.cc

LCFLAGS = \
          -I $(TOPDIR)/../nic/build/${BUILD_ARCH}/iris/gen/proto/ \
          -I $(TOPDIR)/../nic/hal/third-party/spdlog/include/ \
          $(NULL)

LCXXFLAGS = $(LCFLAGS)

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
