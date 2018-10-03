#
# Copyright (c) 2018, Pensando Systems Inc.
#

LIBRARY = libnicmgr.so
SOURCES = hal_client.cc dev.cc eth_dev.cc accel_dev.cc logger.cc


LCFLAGS = \
	-I ../include \
	-I $(TOPDIR) \
	-I $(TOPDIR)/.. \
	-I $(TOPDIR)/include/common \
	-I $(TOPDIR)/src/lib/pal/include \
	-I $(TOPDIR)/src/lib/misc/include \
	-I $(TOPDIR)/src/lib/pcieport/include \
	-I $(TOPDIR)/src/lib/cfgspace/include \
	-I $(TOPDIR)/src/lib/pciehost/include \
	-I $(TOPDIR)/src/lib/pciehw/include \
	-I $(TOPDIR)/src/lib/pciehdevices/include \
	-I $(TOPDIR)/src/lib/intrutils/include \
	-I $(TOPDIR)/include/common/drivers/linux/eth/ionic/ \
	-I $(TOPDIR)/drivers/linux/eth/ionic/ \
	-I $(TOPDIR)/../nic/include/ \
	-I $(TOPDIR)/../nic/sdk/ \
	-I $(TOPDIR)/../nic/sdk/include/ \
	-I $(TOPDIR)/../nic/build/${BUILD_ARCH}/iris/gen/proto/ \
	-I $(TOPDIR)/../nic/build/${BUILD_ARCH}/iris/ \
	-I $(TOPDIR)/../nic/hal/pd/capri/ \
	-I $(TOPDIR)/../nic/third-party/boost/include/ \
	-I $(TOPDIR)/../nic/hal/third-party/include/ \
	-I $(TOPDIR)/../nic/hal/third-party/grpc/include/ \
	-I $(TOPDIR)/../nic/hal/third-party/google/include/ \
	-I $(TOPDIR)/../nic/hal/third-party/spdlog/include/ \
	-I $(TOPDIR)/../nic/model_sim/include/ \
	-I $(TOPDIR)/../nic/asic/capri/model/cap_top \
	-I $(TOPDIR)/../nic/asic/capri/model/cap_top/csr_defines \
	-I $(TOPDIR)/../nic/asic/capri/model/cap_he/readonly \
	-I $(TOPDIR)/../nic/asic/capri/design/common \
	$(NULL)

LCXXFLAGS = $(LCFLAGS)

LIBRARIES = \
	-lmisc \
	-lpciehost \
	-lpciehdevices \
	-lpciehw \
	-lintrutils \
	-lcfgspace \
	-lpcieport \
	-lpcietlp \
	$(NULL)

LLDFLAGS = \
	-L $(TOPDIR)/../nic/obj \
	-L $(TOPDIR)/../nic/conf/sdk \
	-L $(TOPDIR)/../nic/gen/$(BUILD_ARCH)/lib/ \
	-L $(TOPDIR)/../nic/hal/third-party/grpc/$(BUILD_ARCH)/lib \
	-L $(TOPDIR)/../nic/hal/third-party/google/$(BUILD_ARCH)/lib \
	-L $(TOPDIR)/../nic/build/${BUILD_ARCH}/iris/lib/ \
	$(NULL)

SYSTEM_LIBRARIES = \
	-lpthread \
	-lgrpc \
	-lgrpc++ \
	-lgrpc_unsecure \
	-lgrpc++_unsecure \
	$(TOPDIR)/../nic/hal/third-party/google/$(BUILD_ARCH)/lib/libprotobuf.so.14 \
	-lhalproto \
	-lindexer \
	-llogger \
	$(NULL)

ifeq ($(BUILD_ARCH), "x86_64")
	SYSTEM_LIBRARIES += -lpal
else
	LIBRARIES += -l libpal
endif

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
