#
# Copyright (c) 2018, Pensando Systems Inc.
#

PROGRAM = nicmgrd
SOURCES = nicmgrd.cc


LCFLAGS = \
	-I ../include \
	-I $(TOPDIR) \
	-I $(TOPDIR)/include/common \
	-I $(TOPDIR)/src/lib/pal/include \
	-I $(TOPDIR)/src/lib/misc/include \
	-I $(TOPDIR)/src/lib/pcieport/include \
	-I $(TOPDIR)/src/lib/cfgspace/include \
	-I $(TOPDIR)/src/lib/pciehost/include \
	-I $(TOPDIR)/src/lib/pciehw/include \
	-I $(TOPDIR)/src/lib/pciehdevices/include \
	-I $(TOPDIR)/src/lib/intrutils/include \
	-I $(TOPDIR)/src/lib/nicmgr/include \
	-I $(TOPDIR)/include/common/drivers/linux/eth/ionic/ \
	-I $(TOPDIR)/drivers/linux/eth/ionic/ \
	-I $(TOPDIR)/../nic/include/ \
	-I $(TOPDIR)/../nic/sdk/include \
	-I $(TOPDIR)/../nic/sdk \
	-I $(TOPDIR)/../nic/gen/proto/hal/ \
	-I $(TOPDIR)/../nic/third-party/boost/include \
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
	-lpthread \
	-lgrpc \
	-lgrpc++ \
	-lgrpc_unsecure \
	-lgrpc++_unsecure \
	$(TOPDIR)/../nic/hal/third-party/google/$(BUILD_ARCH)/lib/libprotobuf.so.14 \
	-lhalproto \
	-lindexer \
	-llogger \
	-ldl \
	$(NULL)

SYSTEM_LIBRARIES += -lpalsdk
LIBRARIES += \
		-lnicmgr \
		-lpal \
		-lmisc \
		-lpciehost \
		-lpciehdevices \
		-lpciehw \
		-lintrutils \
		-lcfgspace \
		-lpcieport \
		-lpcietlp \
		$(NULL)

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
