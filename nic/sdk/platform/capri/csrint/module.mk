# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = libsdkcapri_csrint.so
MODULE_DEFS     = ${NIC_CSR_DEFINES}
MODULE_FLAGS    = -DCAPRI_SW ${NIC_CSR_FLAGS}
MODULE_SOLIBS   := sdkcapri_asicrw_if
MODULE_EXCLUDE_FLAGS = -O2
MODULE_INCS     = ${NIC_CSR_INCS} ${MODULE_DIR}
MODULE_SRCS     = ${SDKDIR}/third-party/asic/capri/model/cap_top/cap_top_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/utils/cap_csr_base.cc \
                  ${SDKDIR}/third-party/asic/capri/model/utils/mem_access.cc \
                  ${SDKDIR}/third-party/asic/ip/verif/pcpp/pen_axi4_slave.cc \
                  ${SDKDIR}/third-party/asic/ip/verif/pcpp/cpp_int_helper.cc \
                  ${SDKDIR}/third-party/asic/ip/verif/pcpp/common_dpi.cc \
                  ${SDKDIR}/third-party/asic/capri/design/common/gen/cap_qstate_decoders.cc \
                  ${SDKDIR}/third-party/asic/ip/verif/pcpp/axi_xn_db.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_ptd/cap_ptd_decoders.cc \
                  ${SDKDIR}/third-party/asic/capri/design/common/gen/cap_axi_decoders.cc \
                  ${SDKDIR}/third-party/asic/ip/verif/pcpp/pen_csr_base.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_prd/cap_pr_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_ptd/cap_pt_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_psp/cap_psp_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_ppa/cap_ppa_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_prd/cap_prd_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_ptd/cap_ptd_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_te/cap_te_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_mpu/cap_mpu_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_pic/cap_pics_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_pic/cap_picc_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_pcie/cap_pxb_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_pcie/cap_pp_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_npv/cap_npv_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_pic/cap_pict_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_txs/cap_txs_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_pb/cap_pbc_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_pb/cap_pbm_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_pb/cap_pbchbmtx_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_pb/cap_pbchbmeth_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_pb/cap_pbchbm_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_pb/cap_pbc_decoders.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_wa/cap_wa_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_ms/cap_ms_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_ms/cap_msr_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_ms/cap_msh_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_ms/cap_mss_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_em/cap_em_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_em/cap_emm_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_em/emmc_SDHOST_Memory_Map.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_ap/cap_ap_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_ap/cap_apb_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_he/readonly/cap_hens_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_he/readonly/cap_hese_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_he/readonly/cap_mpns_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_he/readonly/cap_mpse_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_pcie/cap_pxp_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_pcie/cap_pxc_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_sema/cap_sema_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_intr/cap_intr_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_bx/cap_bx_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_mx/cap_mx_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_mc/cap_mc_csr.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_mc/cap_mch_csr.cc \
                  ${SDKDIR}/third-party/asic/ip/verif/pcpp/LogMsg.cc \
                  ${SDKDIR}/third-party/asic/ip/verif/pcpp/msg_man.cc \
                  ${SDKDIR}/third-party/asic/ip/verif/pcpp/msg_stream.cc \
                  ${SDKDIR}/third-party/asic/capri/model/utils/cap_csr_py_if.cc \
                  ${SDKDIR}/third-party/asic/ip/verif/pcpp/cpu.cc \
                  ${SDKDIR}/third-party/asic/ip/verif/pcpp/pknobs.cc \
                  ${SDKDIR}/third-party/asic/capri/verif/apis/gen/cap_pb_api.cc \
                  ${SDKDIR}/third-party/asic/capri/verif/apis/gen/cap_quiesce_api.cc \
                  ${SDKDIR}/third-party/asic/capri/verif/apis/cap_txs_api.cc \
                  ${SDKDIR}/third-party/asic/capri/verif/apis/cap_nx_api.cc \
                  ${SDKDIR}/third-party/asic/capri/design/common/gen/cap_phv_intr_decoders.cc \
                  ${SDKDIR}/third-party/asic/capri/verif/apis/cap_npv_api.cc \
                  ${SDKDIR}/third-party/asic/capri/verif/apis/cap_dpa_api.cc \
                  ${SDKDIR}/third-party/asic/capri/verif/apis/cap_intr_api.cc \
                  ${SDKDIR}/third-party/asic/capri/verif/apis/cap_pics_api.cc \
                  ${SDKDIR}/third-party/asic/capri/verif/apis/cap_pict_api.cc \
                  ${SDKDIR}/third-party/asic/capri/verif/apis/cap_ppa_api.cc \
                  ${SDKDIR}/third-party/asic/capri/verif/apis/cap_prd_api.cc \
                  ${SDKDIR}/third-party/asic/capri/verif/apis/cap_psp_api.cc \
                  ${SDKDIR}/third-party/asic/capri/verif/apis/cap_ptd_api.cc \
                  ${SDKDIR}/third-party/asic/capri/verif/apis/cap_stg_api.cc \
                  ${SDKDIR}/third-party/asic/capri/verif/apis/cap_wa_api.cc \
                  ${SDKDIR}/third-party/asic/capri/design/common/gen/cap_lif_qstate_decoders.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_ppa/cap_ppa_decoders.cc \
                  ${SDKDIR}/third-party/asic/capri/verif/apis/cap_csr_util_api.cc \
                  ${SDKDIR}/third-party/asic/capri/model/cap_pic/cap_pic_decoders.cc \
                  ${SDKDIR}/third-party/asic/capri/verif/apis/cap_te_hal_api.cc \
                  ${SDKDIR}/third-party/asic/capri/verif/apis/cap_mpu_api.cc \
                  ${SDKDIR}/third-party/asic/ip/verif/pcpp/pknobs_reader.cc \
                  ${SDKDIR}/third-party/asic/capri/verif/apis/cap_nwl_sbus_api.cc \
                  ${SDKDIR}/third-party/asic/capri/verif/apis/cap_sbus_api.cc \
		  ${SDKDIR}/platform/capri/csrint/csr_init.cc \
                  $(wildcard ${SDKDIR}/third-party/asic/capri/model/cap_dpa/*csr.cc) \
                  $(wildcard ${SDKDIR}/third-party/asic/capri/model/cap_pb/*port*csr.cc)
include ${MKDEFS}/post.mk
