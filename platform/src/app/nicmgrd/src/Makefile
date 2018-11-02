#
# Copyright (c) 2018, Pensando Systems Inc.
#

PROGRAM = nicmgrd
SOURCES = nicmgrd.cc nicmgr_client_reg.cc delphic.cc

LCFLAGS = \
	-I ../include \
	-I $(TOPDIR) \
	-I $(TOPDIR)/.. \
	-I $(TOPDIR)/src/lib/pal/include \
	-I $(TOPDIR)/src/lib/evutils/include \
	-I $(TOPDIR)/src/lib/pcieport/include \
	-I $(TOPDIR)/src/lib/pciemgr/include \
	-I $(TOPDIR)/src/lib/pciemgr_if/include \
	-I $(TOPDIR)/src/lib/pciemgrutils/include \
	-I $(TOPDIR)/src/lib/pciehdevices/include \
	-I $(TOPDIR)/src/lib/nicmgr/include \
	-I $(TOPDIR)/../nic/delphi/sdk/ \
	-I $(TOPDIR)/src/lib/upgrade \
	-I $(TOPDIR)/src/lib/sysmgr \
	-I $(TOPDIR)/../nic/upgrade_manager/export/upgcsdk/ \
	-I $(TOPDIR)/src/app/nicmgrd/src/ \
	-I $(TOPDIR)/include/common/drivers/linux/eth/ionic/ \
	-I $(TOPDIR)/drivers/linux/eth/ionic/ \
	-I $(TOPDIR)/../nic/include/ \
	-I $(TOPDIR)/../nic/third-party/libev/include \
	-I $(TOPDIR)/../nic/sdk/include \
	-I $(TOPDIR)/../nic/sdk \
	-I $(TOPDIR)/../nic/build/${BUILD_ARCH}/iris/gen/proto/ \
	-I $(TOPDIR)/../nic/build/${BUILD_ARCH}/iris/gen/proto/nicmgr \
	-I $(TOPDIR)/../nic/build/${BUILD_ARCH}/iris/gen/proto/common \
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
	-lcapri_csr_cpu_hal \
	-lisa \
	-lcapisa \
	-lcrypto \
	-lhal_mock \
	-lutils \
	-lmtrack \
	-ltrace \
	-lp4pd \
	-lp4pd_common_p4plus_rxdma \
	-lp4pd_common_p4plus_txdma \
	-lasicpd \
	-lcapri_csrint \
	-llist \
	-lbm_allocator \
	-lslab \
	-lthread \
	-lshmmgr \
	-lsknobs \
	-lpdcommon \
	-lp4pd_iris \
	-lpdcapri \
	-lmpuobj \
	-lsdkpal \
	-lsdkplatformutils \
	-lrt \
	-ldirectmap \
	-lht \
	-ldelphisdk \
	-lcommonproto \
	-lnicmgrproto \
	-lupgrade_app \
	-lupgradeutils \
	-lupgrade \
	-lsysmgr \
	-lev \
	$(NULL)

LIBRARIES += \
	-lnicmgr \
	-lnicmgr_sysmgr \
	-lnicmgr_upgrade \
	-lhal_api \
	-lhal_api_trace \
	-lpal \
	-lmisc \
	-lpciehdevices \
	-lpciemgr \
	-lpciemgr_if \
	-lpciemgrutils \
	-lintrutils \
	-levutils \
	-lcfgspace \
	-lpcieport \
	-lpcietlp \
	$(NULL)

ifeq ($(BUILD_ARCH), x86_64)
	SYSTEM_LIBRARIES += -lJudy
else
	LIBRARIES += $(TOPDIR)/../nic/hal/third-party/judy/aarch64/lib/libJudy.so.1
endif

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
