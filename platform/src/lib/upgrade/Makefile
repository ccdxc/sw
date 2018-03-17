#
# Copyright (c) 2018, Pensando Systems Inc.
#

LIBRARY= libnicmgr_upgrade.so 
SOURCES = upgrade.cc 

LCFLAGS = \
	-I ../include \
	-I $(TOPDIR) \
	-I $(TOPDIR)/.. \
	-I $(TOPDIR)/include/common \
	-I $(TOPDIR)/src/lib/nicmgr/include \
    -I $(TOPDIR)/../nic/delphi/sdk/ \
    -I $(TOPDIR)/../nic/upgrade_manager/export/upgcsdk/ \
    -I $(TOPDIR)/src/lib/pal/include \
    -I $(TOPDIR)/src/lib/misc/include \
    -I $(TOPDIR)/src/lib/pcieport/include \
	-I $(TOPDIR)/src/lib/pciemgr/include \
	-I $(TOPDIR)/src/lib/pciemgrutils/include \
    -I $(TOPDIR)/src/lib/pciehdevices/include \
    -I $(TOPDIR)/src/lib/intrutils/include \
    -I $(TOPDIR)/src/lib/nicmgr/include \
    -I $(TOPDIR)/src/lib/delphi \
    -I $(TOPDIR)/src/lib/upgrade \
    -I $(TOPDIR)/src/lib/sysmgr \
    -I $(TOPDIR)/src/lib/misc/include \
    -I $(TOPDIR)/include/common/drivers/linux/eth/ionic/ \
    -I $(TOPDIR)/drivers/linux/eth/ionic/ \
	-I $(TOPDIR)/../nic/include/ \
	-I $(TOPDIR)/../nic/third-party/libev/include \
	-I $(TOPDIR)/../nic/sdk/include \
	-I $(TOPDIR)/../nic/sdk \
	-I $(TOPDIR)/../nic/build/${BUILD_ARCH}/iris/gen/proto/ \
	-I $(TOPDIR)/../nic/build/${BUILD_ARCH}/iris/gen/proto/nicmgr/ \
	-I $(TOPDIR)/../nic/build/${BUILD_ARCH}/iris/gen/proto/common/ \
	-I $(TOPDIR)/../nic/build/${BUILD_ARCH}/iris/ \
	-I $(TOPDIR)/../nic/third-party/boost/include \
	-I $(TOPDIR)/../nic/hal/third-party/include/ \
	-I $(TOPDIR)/../nic/hal/third-party/grpc/include/ \
	-I $(TOPDIR)/../nic/hal/third-party/google/include/ \
	-I $(TOPDIR)/../nic/model_sim/include/ \
	-I $(TOPDIR)/src/lib/hal_api/include \
	-I $(TOPDIR)/../nic/hal/third-party/spdlog/include/ \
	$(NULL)

LCXXFLAGS = $(LCFLAGS)

LLDFLAGS = \
	-L $(TOPDIR)/../nic/obj \
	-L $(TOPDIR)/../nic/conf/sdk \
	-L $(TOPDIR)/../nic/hal/third-party/grpc/$(BUILD_ARCH)/lib \
	-L $(TOPDIR)/../nic/hal/third-party/google/$(BUILD_ARCH)/lib \
	-L $(TOPDIR)/../nic/build/${BUILD_ARCH}/iris/lib \
	$(NULL)

TOPDIR = ../../..
include $(TOPDIR)/make/Makefile.inc
