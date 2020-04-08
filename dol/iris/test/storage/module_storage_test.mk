# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = storage_test.gtest
MODULE_ARCH     = x86_64
MODULE_PIPELINE = iris
MODULE_SRCS     = ${MODULE_SRC_DIR}/main.cc
MODULE_SOLIBS   = storage_hal_if storage_nicmgr_if storage_qstate_if \
                  storage_tests storage_rdma storage_compression \
                  storage_acc_ring storage_acc_scale_tests \
                  storage_comp_encrypt_chain storage_comp_hash_chain \
                  storage_chksum_decomp_chain storage_decrypt_decomp_chain \
                  storage_utils storage_r2n storage_ssd_core \
                  storage_encrypt_only storage_nvme_dp \
                  host_mem storage_qstate_if pal model_client \
                  p4pd_pack_storage_seq \
                  sdkplatformutils shmmgr logger eth_p4plus \
                  nicmgr mnet \
                  pciemgr_if pciemgr pciemgrutils pciehdevices pcietlp cfgspace \
                  intrutils pal misc evutils \
                  ${NIC_SDK_SOLIBS} trace bm_allocator \
                  commonproto nicmgrproto halproto \
                  delphisdk \
                  upgrade upgrade_app upgradeutils \
                  sysmgr \
                  sdkcapri_asicrw_if \
                  sdkasicpd pdcommon hal_mock \
                  sdkp4 sdkp4utils sdkxcvrdriver sdkfru p4pd_${PIPELINE} \
                  p4pd_common_p4plus_rxdma p4pd_common_p4plus_txdma \
                  asicpd ${NIC_HAL_PD_SOLIBS_${ARCH}} devapi_iris rdmamgr_iris hal_devapi
MODULE_LDLIBS   := crypto ${NIC_COMMON_LDLIBS} \
                   ${NIC_THIRDPARTY_GOOGLE_LDLIBS} \
                   ${SDK_THIRDPARTY_CAPRI_LDLIBS} \
                   ${NIC_CAPSIM_LDLIBS} gflags
MODULE_PIPELINE = iris
MODULE_INCS     = ${TOPDIR}/nic/third-party/gflags/include \
                  ${TOPDIR}/nic/hal/third-party/grpc/include \
                  ${TOPDIR}/nic/hal/third-party/google/include \
				  ${TOPDIR}/nic/sdk/include/sdk \
                  ${TOPDIR}/platform/src/lib/nicmgr/include \
                  ${TOPDIR}/platform/include/common \
                  ${TOPDIR}/platform/src/lib/pciemgr/include \
                  ${TOPDIR}/platform/src/lib/pciemgrutils/include \
                  ${TOPDIR}/platform/src/lib/pciehdevices/include \
                  ${TOPDIR}/platform/src/lib/pcieport/include \
                  ${BLD_GEN_DIR}/proto \
                  ${TOPDIR}/nic/sdk/include \
                  ${TOPDIR}/nic/include
include ${MKDEFS}/post.mk
