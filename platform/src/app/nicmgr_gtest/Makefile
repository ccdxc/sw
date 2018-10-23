#
# Copyright (c) 2018, Pensando Systems Inc.
#

LBUILD_ARCHES = x86_64
PROGRAM = nicmgr_test
SOURCES = nicmgr_test.cc

GTEST_ROOT = $(TOPDIR)/../nic/sdk/third-party/googletest-release-1.8.0/googletest
LCFLAGS = \
    -I $(TOPDIR)/src/lib/nicmgr/include \
    -I $(GTEST_ROOT)/include \
    -I $(TOPDIR)/src/lib/hal_api/include \
    -I $(TOPDIR)/src/lib/hal_api/trace \
    -I $(TOPDIR)/include/common \
    -I $(TOPDIR)/src/lib/pal/include \
    -I $(TOPDIR)/src/lib/misc/include \
    -I $(TOPDIR)/src/lib/cfgspace/include \
    -I $(TOPDIR)/src/lib/pciehost/include \
    -I $(TOPDIR)/src/lib/pciehdevices/include \
    -I $(TOPDIR)/src/lib/pciehw/include \
    -I $(TOPDIR)/src/lib/pcieport/include \
    -I $(TOPDIR)/drivers/linux/eth/ionic \
    -I $(TOPDIR)/../nic/include \
    -I $(TOPDIR)/../nic/sdk \
    -I $(TOPDIR)/../nic/sdk/include \
    -I $(TOPDIR)/../nic/build/${BUILD_ARCH}/iris/gen/proto \
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
    -lhal_api_trace \
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

ifeq ($(BUILD_ARCH), x86_64)
	SYSTEM_LIBRARIES += -lJudy
else
	LIBRARIES += $(TOPDIR)/../nic/hal/third-party/judy/aarch64/lib/libJudy.so.1
endif

TOPDIR = ../../..
include $(TOPDIR)/make/Makefile.inc
