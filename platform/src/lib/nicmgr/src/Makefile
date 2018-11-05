#
# Copyright (c) 2018, Pensando Systems Inc.
#

LIBRARY = libnicmgr.so
SOURCES = hal_client.cc pd_client.cc capri_qstate.cc lif_manager.cc dev.cc eth_dev.cc accel_dev.cc logger.cc


LCFLAGS = \
	-I ../include \
	-I $(TOPDIR) \
	-I $(TOPDIR)/.. \
	-I $(TOPDIR)/src/lib/hal_api/include \
	-I $(TOPDIR)/include/common \
	-I $(TOPDIR)/src/third-party/lib/gmp/include \
	-I $(TOPDIR)/src/lib/pal/include \
	-I $(TOPDIR)/src/lib/pciemgr/include \
	-I $(TOPDIR)/src/lib/pciemgr_if/include \
	-I $(TOPDIR)/src/lib/pciemgrutils/include \
	-I $(TOPDIR)/src/lib/pciehdevices/include \
	-I $(TOPDIR)/src/lib/intrutils/include \
	-I $(TOPDIR)/src/lib/upgrade \
	-I $(TOPDIR)/src/lib/sysmgr \
	-I $(TOPDIR)/src/lib/mnet/include \
    -I $(TOPDIR)/../nic/delphi/sdk/ \
	-I $(TOPDIR)/src/app/nicmgrd/src/ \
    -I $(TOPDIR)/../nic/upgrade_manager/export/upgcsdk/ \
	-I $(TOPDIR)/include/common/drivers/linux/eth/ionic/ \
	-I $(TOPDIR)/drivers/linux/eth/ionic/ \
	-I $(TOPDIR)/../nic/include/ \
	-I $(TOPDIR)/../nic/sdk \
	-I $(TOPDIR)/../nic/sdk/include \
	-I $(TOPDIR)/../nic/sdk/include/sdk \
	-I $(TOPDIR)/../nic/sdk/lib/table/directmap \
	-I $(TOPDIR)/../nic/build/${BUILD_ARCH}/iris/gen/proto/ \
	-I $(TOPDIR)/../nic/build/${BUILD_ARCH}/iris/gen/proto/nicmgr \
	-I $(TOPDIR)/../nic/build/${BUILD_ARCH}/iris/gen/proto/common \
	-I $(TOPDIR)/../nic/build/${BUILD_ARCH}/iris/ \
	-I $(TOPDIR)/../nic/build/${BUILD_ARCH}/iris/gen/p4gen/common_rxdma_actions/include \
	-I $(TOPDIR)/../nic/build/${BUILD_ARCH}/iris/gen/p4gen/common_txdma_actions/include \
	-I $(TOPDIR)/../nic/hal/pd/capri \
	-I $(TOPDIR)/../nic/third-party/boost/include/ \
	-I $(TOPDIR)/../nic/third-party/libev/include/ \
	-I $(TOPDIR)/../nic/hal/third-party/include/ \
	-I $(TOPDIR)/../nic/hal/third-party/grpc/include/ \
	-I $(TOPDIR)/../nic/hal/third-party/google/include/ \
	-I $(TOPDIR)/../nic/hal/third-party/spdlog/include/ \
	-I $(TOPDIR)/../nic/model_sim/include/ \
	-I $(TOPDIR)/../nic/asic/ip/verif/pcpp \
	-I $(TOPDIR)/../nic/asic/capri/model/utils \
	-I $(TOPDIR)/../nic/asic/capri/model/cap_top \
	-I $(TOPDIR)/../nic/asic/capri/model/cap_top/csr_defines \
	-I $(TOPDIR)/../nic/asic/capri/model/cap_he/readonly \
	-I $(TOPDIR)/../nic/asic/capri/model/cap_ppa \
	-I $(TOPDIR)/../nic/asic/capri/model/cap_pic \
	-I $(TOPDIR)/../nic/asic/capri/model/cap_wa \
	-I $(TOPDIR)/../nic/asic/capri/model/cap_mpu \
	-I $(TOPDIR)/../nic/asic/capri/model/cap_pbm \
	-I $(TOPDIR)/../nic/asic/capri/model/cap_pcie \
	-I $(TOPDIR)/../nic/asic/capri/model/cap_te \
	-I $(TOPDIR)/../nic/asic/capri/model/cap_txs \
	-I $(TOPDIR)/../nic/asic/capri/model/cap_he/readonly \
	-I $(TOPDIR)/../nic/asic/capri/model/cap_ms \
	-I $(TOPDIR)/../nic/asic/capri/model/cap_ap \
	-I $(TOPDIR)/../nic/asic/capri/model/cap_ptd \
	-I $(TOPDIR)/../nic/asic/capri/model/cap_bx \
	-I $(TOPDIR)/../nic/asic/capri/model/cap_em \
	-I $(TOPDIR)/../nic/asic/capri/model/cap_mc \
	-I $(TOPDIR)/../nic/asic/capri/model/cap_pb \
	-I $(TOPDIR)/../nic/asic/capri/model/cap_intr \
	-I $(TOPDIR)/../nic/asic/capri/model/cap_ms \
	-I $(TOPDIR)/../nic/asic/capri/model/cap_prd \
	-I $(TOPDIR)/../nic/asic/capri/model/cap_sema \
	-I $(TOPDIR)/../nic/asic/capri/model/cap_dpa \
	-I $(TOPDIR)/../nic/asic/capri/model/cap_mx \
	-I $(TOPDIR)/../nic/asic/capri/model/cap_psp \
	-I $(TOPDIR)/../nic/asic/capri/design/common \
	-I $(TOPDIR)/../nic/hal/pd/p4pd/ \
	-I $(TOPDIR)/../nic/hal/pd/iris/internal \
	$(NULL)

LCXXFLAGS = $(LCFLAGS)

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc
