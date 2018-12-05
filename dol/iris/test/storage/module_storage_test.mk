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
                  host_mem storage_qstate_if model_client \
                  halproto sdkp4utils e2e_driver p4pd_pack_storage_seq \
                  sdkplatformutils shmmgr logger
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} \
                  ${NIC_COMMON_LDLIBS} gflags
MODULE_INCS     = ${TOPDIR}/nic/third-party/gflags/include \
                  ${TOPDIR}/nic/hal/third-party/grpc/include \
                  ${TOPDIR}/nic/hal/third-party/google/include \
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
