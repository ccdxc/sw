#
# Copyright (c) 2018, Pensando Systems Inc.
#

LBUILD_ARCHES = x86_64
PROGRAM = nicmgr_test
SOURCES = nicmgr_test.cc

GTEST_ROOT = $(TOPDIR)/../nic/sdk/third-party/googletest-release-1.8.0/googletest
LCFLAGS = \
    -I $(TOPDIR)/src/lib/nicmgr/include \
    -I $(TOPDIR)/.. \
    -I $(TOPDIR)/src/app/nicmgrd/src \
    -I $(TOPDIR)/../nic/upgrade_manager/export/upgcsdk/ \
    -I $(TOPDIR)/../nic/delphi/sdk/ \
    -I $(TOPDIR)/src/lib/upgrade \
    -I $(GTEST_ROOT)/include \
    -I $(TOPDIR)/src/lib/hal_api/include \
    -I $(TOPDIR)/src/lib/hal_api/trace \
    -I $(TOPDIR)/include/common \
    -I $(TOPDIR)/src/lib/pal/include \
    -I $(TOPDIR)/src/lib/misc/include \
    -I $(TOPDIR)/src/lib/mnet/include \
    -I $(TOPDIR)/src/lib/pcieport/include \
    -I $(TOPDIR)/src/lib/cfgspace/include \
    -I $(TOPDIR)/src/lib/pciemgr/include \
    -I $(TOPDIR)/src/lib/pciemgr_if/include \
    -I $(TOPDIR)/src/lib/pciemgrutils/include \
    -I $(TOPDIR)/src/lib/pciehdevices/include \
    -I $(TOPDIR)/drivers/linux/eth/ionic \
    -I $(TOPDIR)/../nic/include \
    -I $(TOPDIR)/../nic/third-party/libev/include \
    -I $(TOPDIR)/../nic/build/${BUILD_ARCHES}/iris  \
    -I $(TOPDIR)/../nic/build/${BUILD_ARCH}/iris  \
    -I $(TOPDIR)/src/lib/sysmgr \
    -I $(TOPDIR)/../nic/sdk \
    -I $(TOPDIR)/../nic/sdk/include \
    -I $(TOPDIR)/../nic/build/${BUILD_ARCH}/iris/gen/proto \
    -I $(TOPDIR)/../nic/build/${BUILD_ARCH}/iris/gen/proto/nicmgr \
    -I $(TOPDIR)/../nic/third-party/boost/include \
    -I $(TOPDIR)/../nic/hal/third-party/spdlog/include/ \
    -I $(TOPDIR)/../nic/hal/third-party/google/include \
    -I $(TOPDIR)/../nic/hal/third-party/grpc/include \
    $(NULL)

LCXXFLAGS = $(LCFLAGS)
LLDFLAGS = \
    -L $(TOPDIR)/../nic/build/${BUILD_ARCH}/iris/lib \
    -L $(TOPDIR)/../nic/hal/third-party/google/$(BUILD_ARCH)/lib \
    -L $(TOPDIR)/../nic/sdk/third-party/googletest-release-1.8.0/googletest/make/$(BUILD_ARCH) \
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
    -ldl \
    -lp4pd \
    -lp4pd_common_p4plus_rxdma \
    -lp4pd_common_p4plus_txdma \
    -lp4pd_iris \
    -lasicpd \
    -lcapri_csrint \
    -lpdcapri \
    -lsdkpal \
    -lsdkcapri \
    -lsdkplatformutils \
    -lcapri_csr_cpu_hal \
    -lmpuobj \
    -lcapisa \
    -lisa \
    -lbm_allocator \
    -lsknobs \
    -lpdcommon \
    -llist \
    -lslab \
    -lcrypto \
    -lutils \
    -lmtrack \
    -ltrace \
    -lhal_mock \
    -lthread \
    -lshmmgr \
    -lcommonproto \
    -lnicmgrproto \
    -lrt \
	-ldirectmap \
	-lht \
	-ldelphisdk \
	-lev \
    $(NULL)
#$(GTEST_ROOT)/make/gtest.a

SYSTEM_LIBRARIES += -lsdkpal
LIBRARIES += \
    -lnicmgr \
    -lhal_api \
    -lpal \
    -lmisc \
    -lpciehdevices \
    -lpciemgr \
    -lpciemgr_if \
    -lpciemgrutils \
    -levutils \
    -lintrutils \
    -lcfgspace \
    -lpcieport \
    -lpcietlp \
	-lmnet \
    $(NULL)

ifeq ($(BUILD_ARCH), x86_64)
	SYSTEM_LIBRARIES += -lJudy
else
	LIBRARIES += $(TOPDIR)/../nic/hal/third-party/judy/aarch64/lib/libJudy.so.1
endif

TOPDIR = ../../..
include $(TOPDIR)/make/Makefile.inc
