#include "asicerrord.h"

struct interrupts dpp_int_credit[] = { 
    {"ptr_credit_ovflow", 0},
    {"ptr_credit_undflow", 0},
    {"pkt_credit_ovflow", 0},
    {"pkt_credit_undflow", 0},
    {"framer_credit_ovflow", 0},
    {"framer_credit_undflow", 0},
    {"framer_hdrfld_vld_ovfl", 0},
    {"framer_hdrfld_offset_ovfl", 0},
    {"err_framer_hdrsize_zero_ovfl", 0},
};

struct interrupts dpp_int_fifo[] = {
    {"phv_ff_overflow", 0},
    {"ohi_ff_overflow", 0},
    {"pkt_size_ff_ovflow", 0},
    {"pkt_size_ff_undflow", 0},
    {"csum_phv_ff_ovflow", 0},
    {"csum_phv_ff_undflow", 0},
};

struct interrupts dpp_int_reg1[] = {
    {"err_phv_sop_no_eop", 0},
    {"err_phv_eop_no_sop", 0},
    {"err_ohi_sop_no_eop", 0},
    {"err_ohi_eop_no_sop", 0},
    {"err_framer_credit_overrun", 0},
    {"err_packets_in_flight_credit_overrun", 0},
    {"err_null_hdr_vld", 0},
    {"err_null_hdrfld_vld", 0},
    {"err_max_pkt_size", 0},
    {"err_max_active_hdrs", 0},
    {"err_phv_no_data_reference_ohi", 0},
    {"err_csum_multiple_hdr", 0},
    {"err_csum_multiple_hdr_copy", 0},
    {"err_crc_multiple_hdr", 0},
    {"err_ptr_fifo_credit_overrun", 0},
    {"err_clip_max_pkt_size", 0},
    {"err_min_pkt_size", 0},
};

struct interrupts dpp_int_reg2[] = {
    {"fieldC", 0},
    {"fieldD", 0},
};

struct interrupts dpp_int_spare[] = {
    {"spare_0", 0},
    {"spare_1", 0},
    {"spare_2", 0},
    {"spare_3", 0},
    {"spare_4", 0},
    {"spare_5", 0},
    {"spare_6", 0},
    {"spare_7", 0},
    {"spare_8", 0},
    {"spare_9", 0},
    {"spare_10", 0},
    {"spare_11", 0},
    {"spare_12", 0},
    {"spare_13", 0},
    {"spare_14", 0},
    {"spare_15", 0},
    {"spare_16", 0},
    {"spare_17", 0},
    {"spare_18", 0},
    {"spare_19", 0},
    {"spare_20", 0},
    {"spare_21", 0},
    {"spare_22", 0},
    {"spare_23", 0},
    {"spare_24", 0},
    {"spare_25", 0},
    {"spare_26", 0},
    {"spare_27", 0},
    {"spare_28", 0},
    {"spare_29", 0},
    {"spare_30", 0},
    {"spare_31", 0},
};

struct interrupts dpp_int_srams_ecc[] = {
    {"dpp_phv_fifo_uncorrectable", 0},
    {"dpp_phv_fifo_correctable", 0},
    {"dpp_ohi_fifo_uncorrectable", 0},
    {"dpp_ohi_fifo_correctable", 0},
};

struct interrupts dpr_int_credit[] = {
    {"egress_credit_ovflow", 0},
    {"egress_credit_undflow", 0},
    {"pktout_credit_ovflow", 0},
    {"pktout_credit_undflow", 0},
};

struct interrupts dpr_int_fifo[] = {
    {"phv_ff_ovflow", 0},
    {"ohi_ff_ovflow", 0},
    {"pktin_ff_ovflow", 0},
    {"pktout_ff_undflow", 0},
    {"csum_ff_ovflow", 0},
    {"ptr_ff_ovflow", 0},
};

struct interrupts dpr_int_flop_fifo[] = {
    {"data_mux_force_bypass_crc_flop_ff_ovflow", 0},
    {"dpr_crc_info_flop_ff_ovflow", 0},
    {"dpr_crc_update_info_flop_ff_ovflow", 0},
    {"dpr_csum_info_flop_ff_ovflow", 0},
    {"data_mux_force_bypass_csum_flop_ff_ovflow", 0},
    {"dpr_csum_update_info_flop_ff_ovflow", 0},
    {"ptr_early_pkt_eop_info_flop_ff_ovflow", 0},
    {"data_mux_eop_err_flop_ff_ovflow", 0},
    {"pktin_eop_err_flop_ff_ovflow", 0},
    {"csum_err_flop_ff_ovflow", 0},
    {"crc_err_flop_ff_ovflow", 0},
    {"data_mux_drop_flop_ff_ovflow", 0},
    {"phv_pkt_data_flop_ff_ovflow", 0},
    {"pktout_len_cell_flop_ff_ovflow", 0},
    {"padding_size_flop_ff_ovflow", 0},
    {"pktin_err_flop_ff_ovflow", 0},
    {"phv_no_data_flop_ff_ovflow", 0},
    {"ptr_lookahaed_flop_ff_ovflow", 0},
    {"eop_vld_flop_ff_ovflow", 0},
    {"csum_cal_vld_flop_ff_ovflow", 0},
};

struct interrupts dpr_int_reg1[] = {
    {"err_phv_sop_no_eop", 0},
    {"err_phv_eop_no_sop", 0},
    {"err_ohi_sop_no_eop", 0},
    {"err_ohi_eop_no_sop", 0},
    {"err_pktin_sop_no_eop", 0},
    {"err_pktin_eop_no_sop", 0},
    {"err_csum_offset_gt_pkt_size_4", 0},
    {"err_csum_offset_gt_pkt_size_3", 0},
    {"err_csum_offset_gt_pkt_size_2", 0},
    {"err_csum_offset_gt_pkt_size_1", 0},
    {"err_csum_offset_gt_pkt_size_0", 0},
    {"err_csum_phdr_offset_gt_pkt_size_4", 0},
    {"err_csum_phdr_offset_gt_pkt_size_3", 0},
    {"err_csum_phdr_offset_gt_pkt_size_2", 0},
    {"err_csum_phdr_offset_gt_pkt_size_1", 0},
    {"err_csum_phdr_offset_gt_pkt_size_0", 0},
    {"err_csum_loc_gt_pkt_size_4", 0},
    {"err_csum_loc_gt_pkt_size_3", 0},
    {"err_csum_loc_gt_pkt_size_2", 0},
    {"err_csum_loc_gt_pkt_size_1", 0},
    {"err_csum_loc_gt_pkt_size_0", 0},
    {"err_crc_offset_gt_pkt_size", 0},
    {"err_crc_loc_gt_pkt_size", 0},
    {"err_crc_mask_offset_gt_pkt_size", 0},
    {"err_pkt_eop_early", 0},
    {"err_ptr_ff_overflow", 0},
    {"err_csum_ff_overflow", 0},
    {"err_pktout_ff_overflow", 0},
    {"err_ptr_from_cfg_overflow", 0},
};

struct interrupts dpr_int_reg2[] = {
    {"fieldC", 0},
    {"fieldD", 0},
    {"err_csum_start_gt_end_4", 0},
    {"err_csum_start_gt_end_3", 0},
    {"err_csum_start_gt_end_2", 0},
    {"err_csum_start_gt_end_1", 0},
    {"err_csum_start_gt_end_0", 0},
    {"err_crc_start_gt_end", 0},
};

struct interrupts dpr_int_spare[] = {
    {"spare_0", 0},
    {"spare_1", 0},
    {"spare_2", 0},
    {"spare_3", 0},
    {"spare_4", 0},
    {"spare_5", 0},
    {"spare_6", 0},
    {"spare_7", 0},
    {"spare_8", 0},
    {"spare_9", 0},
    {"spare_10", 0},
    {"spare_11", 0},
    {"spare_12", 0},
    {"spare_13", 0},
    {"spare_14", 0},
    {"spare_15", 0},
    {"spare_16", 0},
    {"spare_17", 0},
    {"spare_18", 0},
    {"spare_19", 0},
    {"spare_20", 0},
    {"spare_21", 0},
    {"spare_22", 0},
    {"spare_23", 0},
    {"spare_24", 0},
    {"spare_25", 0},
    {"spare_26", 0},
    {"spare_27", 0},
    {"spare_28", 0},
    {"spare_29", 0},
    {"spare_30", 0},
    {"spare_31", 0},
};

struct interrupts dpr_int_srams_ecc[] = {
    {"dpr_pktin_fifo_uncorrectable", 0},
    {"dpr_pktin_fifo_correctable", 0},
    {"dpr_csum_fifo_uncorrectable", 0},
    {"dpr_csum_fifo_correctable", 0},
    {"dpr_phv_fifo_uncorrectable", 0},
    {"dpr_phv_fifo_correctable", 0},
    {"dpr_ohi_fifo_uncorrectable", 0},
    {"dpr_ohi_fifo_correctable", 0},
    {"dpr_ptr_fifo_uncorrectable", 0},
    {"dpr_ptr_fifo_correctable", 0},
    {"dpr_pktout_fifo_uncorrectable", 0},
    {"dpr_pktout_fifo_correctable", 0},
};

struct interrupts sse_pics_int_badaddr[] = {
    {"rdreq0_bad_addr", 0},
    {"wrreq0_bad_addr", 0},
    {"rdreq1_bad_addr", 0},
    {"wrreq1_bad_addr", 0},
    {"rdreq2_bad_addr", 0},
    {"wrreq2_bad_addr", 0},
    {"rdreq3_bad_addr", 0},
    {"wrreq3_bad_addr", 0},
    {"rdreq4_bad_addr", 0},
    {"wrreq4_bad_addr", 0},
    {"rdreq5_bad_addr", 0},
    {"wrreq5_bad_addr", 0},
    {"rdreq6_bad_addr", 0},
    {"wrreq6_bad_addr", 0},
    {"rdreq7_bad_addr", 0},
    {"wrreq7_bad_addr", 0},
    {"cpu_bad_addr", 0},
    {"bg_bad_addr", 0},
};

struct interrupts sse_pics_int_bg[] = {
    {"unfinished_bg0", 0},
    {"unfinished_bg1", 0},
    {"unfinished_bg2", 0},
    {"unfinished_bg3", 0},
    {"unfinished_bg4", 0},
    {"unfinished_bg5", 0},
    {"unfinished_bg6", 0},
    {"unfinished_bg7", 0},
    {"unfinished_bg8", 0},
    {"unfinished_bg9", 0},
    {"unfinished_bg10", 0},
    {"unfinished_bg11", 0},
    {"unfinished_bg12", 0},
    {"unfinished_bg13", 0},
    {"unfinished_bg14", 0},
    {"unfinished_bg15", 0},
};

struct interrupts sse_pics_int_pics[] = {
    {"uncorrectable_ecc", 0},
    {"correctable_ecc", 0},
    {"too_many_rl_sch_error", 0},
};

struct interrupts db_wa_int_db[] = {
    {"db_cam_conflict", 0},
    {"db_pid_chk_fail", 0},
    {"db_qid_overflow", 0},
    {"host_ring_access_err", 0},
    {"total_ring_access_err", 0},
    {"rresp_err", 0},
    {"bresp_err", 0},
};

struct interrupts db_wa_int_lif_qstate_map[] = {
    {"ecc_uncorrectable", 0},
    {"ecc_correctable", 0},
    {"qid_invalid", 0},
};

struct interrupts sge_te_int_err[] = {
    {"miss_sop", 0},
    {"miss_eop", 0},
    {"phv_max_size", 0},
    {"spurious_axi_rsp", 0},
    {"spurious_tcam_rsp", 0},
    {"te2mpu_timeout", 0},
    {"axi_rdrsp_err", 0},
    {"axi_bad_read", 0},
    {"tcam_req_idx_fifo", 0},
    {"tcam_rsp_idx_fifo", 0},
    {"mpu_req_idx_fifo", 0},
    {"axi_req_idx_fifo", 0},
    {"proc_tbl_vld_wo_proc", 0},
    {"pend_wo_wb", 0},
    {"pend1_wo_pend0", 0},
    {"both_pend_down", 0},
    {"pend_wo_proc_down", 0},
    {"both_pend_went_up", 0},
    {"loaded_but_no_proc", 0},
    {"loaded_but_no_proc_tbl_vld", 0},
};

struct interrupts sge_te_int_info[] = {
    {"profile_cam_hit0", 0},
    {"profile_cam_hit1", 0},
    {"profile_cam_hit2", 0},
    {"profile_cam_hit3", 0},
    {"profile_cam_hit4", 0},
    {"profile_cam_hit5", 0},
    {"profile_cam_hit6", 0},
    {"profile_cam_hit7", 0},
    {"profile_cam_hit8", 0},
    {"profile_cam_hit9", 0},
    {"profile_cam_hit10", 0},
    {"profile_cam_hit11", 0},
    {"profile_cam_hit12", 0},
    {"profile_cam_hit13", 0},
    {"profile_cam_hit14", 0},
    {"profile_cam_hit15", 0},
    {"profile_cam_miss", 0},
};

struct interrupts sge_mpu_int_err[] = {
    {"results_mismatch", 0},
    {"sdp_mem_uncorrectable", 0},
    {"sdp_mem_correctable", 0},
    {"illegal_op_0", 0},
    {"illegal_op_1", 0},
    {"illegal_op_2", 0},
    {"illegal_op_3", 0},
    {"max_inst_0", 0},
    {"max_inst_1", 0},
    {"max_inst_2", 0},
    {"max_inst_3", 0},
    {"phvwr_0", 0},
    {"phvwr_1", 0},
    {"phvwr_2", 0},
    {"phvwr_3", 0},
    {"write_err_0", 0},
    {"write_err_1", 0},
    {"write_err_2", 0},
    {"write_err_3", 0},
    {"cache_axi_0", 0},
    {"cache_axi_1", 0},
    {"cache_axi_2", 0},
    {"cache_axi_3", 0},
    {"cache_parity_0", 0},
    {"cache_parity_1", 0},
    {"cache_parity_2", 0},
    {"cache_parity_3", 0},
};

struct interrupts sge_mpu_int_info[] = {
    {"trace_full_0", 0},
    {"trace_full_1", 0},
    {"trace_full_2", 0},
    {"trace_full_3", 0},
    {"mpu_stop_0", 0},
    {"mpu_stop_1", 0},
    {"mpu_stop_2", 0},
    {"mpu_stop_3", 0},
};

struct interrupts md_hens_int_axi_err[] = {
    {"wrsp_err", 0},
    {"rrsp_err", 0},
};

struct interrupts md_hens_int_drbg_cryptoram_ecc[] = {
    {"correctable_err", 0},
    {"uncorrectable_err", 0},
};

struct interrupts md_hens_int_drbg_intram_ecc[] = {
    {"correctable_err", 0},
    {"uncorrectable_err", 0},
};

struct interrupts md_hens_int_ipcore[] = {
    {"xts_enc", 0},
    {"xts", 0},
    {"gcm0", 0},
    {"gcm1", 0},
    {"drbg", 0},
    {"pk", 0},
    {"cp", 0},
    {"dc", 0},
};

struct interrupts md_hens_int_pk_ecc[] = {
    {"correctable_err", 0},
    {"uncorrectable_err", 0},
};

struct interrupts mp_mpns_int_crypto[] = {
    {"mpp0", 0},
    {"mpp1", 0},
    {"mpp2", 0},
    {"mpp3", 0},
    {"mpp4", 0},
    {"mpp5", 0},
    {"mpp6", 0},
    {"mpp7", 0},
};

struct interrupts pb_pbc_int_credit_underflow[] = {
    {"port_10", 0},
    {"port_11", 0},
};

struct interrupts pb_pbc_int_ecc_desc[] = {
    {"uncorrectable", 0},
    {"correctable", 0},
};

struct interrupts pb_pbc_int_ecc_fc[] = {
    {"uncorrectable", 0},
    {"correctable", 0},
};

struct interrupts pb_pbc_int_ecc_ll[] = {
    {"uncorrectable", 0},
    {"correctable", 0},
};

struct interrupts pb_pbc_int_ecc_nc[] = {
    {"uncorrectable", 0},
    {"correctable", 0},
};

struct interrupts pb_pbc_int_ecc_pack[] = {
    {"uncorrectable", 0},
    {"correctable", 0},
};

struct interrupts pb_pbc_int_ecc_port_mon[] = {
    {"uncorrectable", 0},
    {"correctable", 0},
};

struct interrupts pb_pbc_int_ecc_rc[] = {
    {"uncorrectable", 0},
    {"correctable", 0},
};

struct interrupts pb_pbc_int_ecc_rwr[] = {
    {"uncorrectable", 0},
    {"correctable", 0},
};

struct interrupts pb_pbc_int_ecc_sched[] = {
    {"uncorrectable", 0},
    {"correctable", 0},
};

struct interrupts pb_pbc_int_ecc_sideband[] = {
    {"uncorrectable", 0},
    {"correctable", 0},
};

struct interrupts pb_pbc_int_ecc_uc[] = {
    {"uncorrectable", 0},
    {"correctable", 0},
};

struct interrupts pb_pbc_int_pbus_violation[] = {
    {"sop_sop_in_0", 0},
    {"sop_sop_in_1", 0},
    {"sop_sop_in_2", 0},
    {"sop_sop_in_3", 0},
    {"sop_sop_in_4", 0},
    {"sop_sop_in_5", 0},
    {"sop_sop_in_6", 0},
    {"sop_sop_in_7", 0},
    {"sop_sop_in_8", 0},
    {"sop_sop_in_9", 0},
    {"sop_sop_in_10", 0},
    {"sop_sop_in_11", 0},
    {"eop_eop_in_0", 0},
    {"eop_eop_in_1", 0},
    {"eop_eop_in_2", 0},
    {"eop_eop_in_3", 0},
    {"eop_eop_in_4", 0},
    {"eop_eop_in_5", 0},
    {"eop_eop_in_6", 0},
    {"eop_eop_in_7", 0},
    {"eop_eop_in_8", 0},
    {"eop_eop_in_9", 0},
    {"eop_eop_in_10", 0},
    {"eop_eop_in_11", 0},
};

struct interrupts pb_pbc_int_rpl[] = {
    {"memory_error", 0},
    {"zero_last_error", 0},
};

struct interrupts pb_pbc_int_write[] = {
    {"out_of_cells", 0},
    {"out_of_credit", 0},
    {"port_disabled", 0},
    {"truncation", 0},
    {"intrinsic_drop", 0},
    {"out_of_cells1", 0},
    {"enq_err", 0},
    {"tail_drop_cpu", 0},
    {"tail_drop_span", 0},
    {"min_size_viol", 0},
    {"port_range", 0},
    {"credit_growth_error", 0},
    {"oq_range", 0},
    {"xoff_timeout", 0},
};

struct interrupts pb_pbc_hbm_int_ecc_hbm_ht[] = {
    {"uncorrectable", 0},
    {"correctable", 0},
};

struct interrupts pb_pbc_hbm_int_ecc_hbm_mtu[] = {
    {"uncorrectable", 0},
    {"correctable", 0},
};

struct interrupts pb_pbc_hbm_int_ecc_hbm_rb[] = {
    {"rb_uncorrectable", 0},
    {"rb_correctable", 0},
    {"cdt_uncorrectable", 0},
    {"cdt_correctable", 0},
};

struct interrupts pb_pbc_hbm_int_ecc_hbm_wb[] = {
    {"uncorrectable", 0},
    {"correctable", 0},
};

struct interrupts pb_pbc_hbm_int_hbm_axi_err_rsp[] = {
    {"ctrl", 0},
    {"pyld", 0},
    {"r2a", 0},
};

struct interrupts pb_pbc_hbm_int_hbm_drop[] = {
    {"occupancy_0", 0},
    {"occupancy_1", 0},
    {"occupancy_2", 0},
    {"occupancy_3", 0},
    {"occupancy_4", 0},
    {"occupancy_5", 0},
    {"occupancy_6", 0},
    {"occupancy_7", 0},
    {"occupancy_8", 0},
    {"occupancy_9", 0},
    {"ctrl_full_0", 0},
    {"ctrl_full_1", 0},
    {"ctrl_full_2", 0},
    {"ctrl_full_3", 0},
    {"ctrl_full_4", 0},
    {"ctrl_full_5", 0},
    {"ctrl_full_6", 0},
    {"ctrl_full_7", 0},
    {"ctrl_full_8", 0},
    {"ctrl_full_9", 0},
};

struct interrupts pb_pbc_hbm_int_hbm_pbus_violation[] = {
    {"sop_sop_in_0", 0},
    {"sop_sop_in_1", 0},
    {"sop_sop_in_2", 0},
    {"sop_sop_in_3", 0},
    {"sop_sop_in_4", 0},
    {"sop_sop_in_5", 0},
    {"sop_sop_in_6", 0},
    {"sop_sop_in_7", 0},
    {"sop_sop_in_8", 0},
    {"sop_sop_in_9", 0},
    {"eop_eop_in_0", 0},
    {"eop_eop_in_1", 0},
    {"eop_eop_in_2", 0},
    {"eop_eop_in_3", 0},
    {"eop_eop_in_4", 0},
    {"eop_eop_in_5", 0},
    {"eop_eop_in_6", 0},
    {"eop_eop_in_7", 0},
    {"eop_eop_in_8", 0},
    {"eop_eop_in_9", 0},
};

struct interrupts pb_pbc_hbm_int_hbm_xoff[] = {
    {"timeout", 0},
};

struct interrupts pm_pbm_int_ecc_col[] = {
    {"uncorrectable", 0},
    {"correctable", 0},
};

struct interrupts mc_mch_int_mc[] = {
    {"ecc_1bit_thres_ps1", 0},
    {"ecc_1bit_thres_ps0", 0},
};

struct asic_registers capri_registers[] = {
    {DPP0_INT_CREDIT, "dpp0_int_credit", 9, FATAL, dpp_int_credit},
    {DPP1_INT_CREDIT, "dpp1_int_credit", 9, FATAL, dpp_int_credit},
    {DPP0_INT_FIFO, "dpp0_int_fifo", 6, FATAL, dpp_int_fifo},
    {DPP1_INT_FIFO, "dpp1_int_fifo", 6, FATAL, dpp_int_fifo},
    {DPP0_INT_REG1, "dpp0_int_reg1", 17, ERROR, dpp_int_reg1},
    {DPP0_INT_REG2, "dpp0_int_reg2", 2, ERROR, dpp_int_reg2},
    {DPP1_INT_REG1, "dpp1_int_reg1", 17, ERROR, dpp_int_reg1},
    {DPP1_INT_REG2, "dpp1_int_reg2", 2, ERROR, dpp_int_reg2},
    {DPP0_INT_SPARE, "dpp0_int_spare", 32, ERROR, dpp_int_spare},
    {DPP1_INT_SPARE, "dpp1_int_spare", 32, ERROR, dpp_int_spare},
    {DPP0_INT_SRAMS_ECC, "dpp0_int_srams_ecc", 4, ERROR, dpp_int_srams_ecc},
    {DPP1_INT_SRAMS_ECC, "dpp1_int_srams_ecc", 4, ERROR, dpp_int_srams_ecc},
    {DPR0_INT_CREDIT, "dpr0_int_credit", 4, FATAL, dpr_int_credit},
    {DPR1_INT_CREDIT, "dpr1_int_credit", 4, FATAL, dpr_int_credit},
    {DPR0_INT_FIFO, "dpr0_int_fifo", 6, FATAL, dpr_int_fifo},
    {DPR1_INT_FIFO, "dpr1_int_fifo", 6, FATAL, dpr_int_fifo},
    {DPR0_INT_FLOP_FIFO, "dpr0_int_flop_fifo_0", 20, FATAL, dpr_int_flop_fifo},
    {DPR1_INT_FLOP_FIFO, "dpr1_int_flop_fifo_1", 20, FATAL, dpr_int_flop_fifo},
    {DPR0_INT_REG1, "dpr0_int_reg1", 29, ERROR, dpr_int_reg1},
    {DPR0_INT_REG2, "dpr0_int_reg2", 8, ERROR, dpr_int_reg2},
    {DPR1_INT_REG1, "dpr1_int_reg1", 29, ERROR, dpr_int_reg1},
    {DPR1_INT_REG2, "dpr1_int_reg2", 8, ERROR, dpr_int_reg2},
    {DPR0_INT_SPARE, "dpr0_int_spare", 32, FATAL, dpr_int_spare},
    {DPR1_INT_SPARE, "dpr1_int_spare", 32, FATAL, dpr_int_spare},
    {DPR0_INT_SRAMS_ECC, "dpr0_int_srams_ecc", 12, ERROR, dpr_int_srams_ecc},
    {DPR1_INT_SRAMS_ECC, "dpr1_int_srams_ecc", 12, ERROR, dpr_int_srams_ecc},
    {SSE_PICS_INT_BADADDR, "sse_pics_int_badaddr", 18, INFO, sse_pics_int_badaddr},
    {SSE_PICS_INT_BG, "sse_pics_int_bg", 16, INFO, sse_pics_int_bg},
    {SSE_PICS_INT_PICS, "sse_pics_int_pics", 3, ERROR, sse_pics_int_pics},
//    {DB_WA_INT_DB, "db_wa_int_db", 7, ERROR, db_wa_int_db},
//    {DB_WA_INT_LIF_QSTATE_MAP, "db_wa_int_lif_qstate_map", 3, ERROR, db_wa_int_lif_qstate_map},
    {SGE_TE0_INT_ERR, "sge_te0_int_err", 20, FATAL, sge_te_int_err},
    {SGE_TE1_INT_ERR, "sge_te1_int_err", 20, FATAL, sge_te_int_err},
    {SGE_TE2_INT_ERR, "sge_te2_int_err", 20, FATAL, sge_te_int_err},
    {SGE_TE3_INT_ERR, "sge_te3_int_err", 20, FATAL, sge_te_int_err},
    {SGE_TE4_INT_ERR, "sge_te4_int_err", 20, FATAL, sge_te_int_err},
    {SGE_TE5_INT_ERR, "sge_te5_int_err", 20, FATAL, sge_te_int_err},
//    {SGE_TE0_INT_INFO, "sge_te0_int_info", 17, INFO, sge_te_int_info},
//    {SGE_TE1_INT_INFO, "sge_te1_int_info", 17, INFO, sge_te_int_info},
//    {SGE_TE2_INT_INFO, "sge_te2_int_info", 17, INFO, sge_te_int_info},
//    {SGE_TE3_INT_INFO, "sge_te3_int_info", 17, INFO, sge_te_int_info},
//    {SGE_TE4_INT_INFO, "sge_te4_int_info", 17, INFO, sge_te_int_info},
//    {SGE_TE5_INT_INFO, "sge_te5_int_info", 17, INFO, sge_te_int_info},
    {SGE_MPU0_INT_ERR, "sge_mpu0_int_err", 27, ERROR, sge_mpu_int_err},
    {SGE_MPU1_INT_ERR, "sge_mpu1_int_err", 27, ERROR, sge_mpu_int_err},
    {SGE_MPU2_INT_ERR, "sge_mpu2_int_err", 27, ERROR, sge_mpu_int_err},
    {SGE_MPU3_INT_ERR, "sge_mpu3_int_err", 27, ERROR, sge_mpu_int_err},
    {SGE_MPU4_INT_ERR, "sge_mpu4_int_err", 27, ERROR, sge_mpu_int_err},
    {SGE_MPU5_INT_ERR, "sge_mpu5_int_err", 27, ERROR, sge_mpu_int_err},
    {SGE_MPU0_INT_INFO, "sge_mpu0_int_info", 8, ERROR, sge_mpu_int_info},
    {SGE_MPU1_INT_INFO, "sge_mpu1_int_info", 8, ERROR, sge_mpu_int_info},
    {SGE_MPU2_INT_INFO, "sge_mpu2_int_info", 8, ERROR, sge_mpu_int_info},
    {SGE_MPU3_INT_INFO, "sge_mpu3_int_info", 8, ERROR, sge_mpu_int_info},
    {SGE_MPU4_INT_INFO, "sge_mpu4_int_info", 8, ERROR, sge_mpu_int_info},
    {SGE_MPU5_INT_INFO, "sge_mpu5_int_info", 8, ERROR, sge_mpu_int_info},
    {MD_HENS_INT_AXI_ERR, "md_hens_int_axi_err", 2, FATAL, md_hens_int_axi_err},
    {MD_HENS_INT_DRBG_CRYPTORAM_ECC, "md_hens_int_drbg_cryptoram_ecc", 2, ERROR, md_hens_int_drbg_cryptoram_ecc},
    {MD_HENS_INT_DRBG_INTRAM_ECC, "md_hens_int_drbg_intram_ecc", 2, ERROR, md_hens_int_drbg_intram_ecc},
    {MD_HENS_INT_IPCORE, "md_hens_int_ipcore", 8, UNKNOWN, md_hens_int_ipcore},
    {MD_HENS_INT_PK_ECC, "md_hens_int_pk_ecc", 2, ERROR, md_hens_int_pk_ecc},
    {MP_MPNS_INT_CRYPTO, "mp_mpns_int_crypto", 8, UNKNOWN, mp_mpns_int_crypto},
    {PB_PBC_INT_CREDIT_UNDERFLOW, "pb_pbc_int_credit_underflow", 2, FATAL, pb_pbc_int_credit_underflow},
    {PB_PBC_INT_ECC_DESC_0, "pb_pbc_int_ecc_desc_0", 2, ERROR, pb_pbc_int_ecc_desc},
    {PB_PBC_INT_ECC_DESC_1, "pb_pbc_int_ecc_desc_1", 2, ERROR, pb_pbc_int_ecc_desc},
    {PB_PBC_INT_ECC_FC_0, "pb_pbc_int_ecc_fc_0", 2, ERROR, pb_pbc_int_ecc_fc},
    {PB_PBC_INT_ECC_FC_1, "pb_pbc_int_ecc_fc_1", 2, ERROR, pb_pbc_int_ecc_fc},
    {PB_PBC_INT_ECC_LL_0, "pb_pbc_int_ecc_ll_0", 2, ERROR, pb_pbc_int_ecc_ll},
    {PB_PBC_INT_ECC_LL_1, "pb_pbc_int_ecc_ll_1", 2, ERROR, pb_pbc_int_ecc_ll},
    {PB_PBC_INT_ECC_NC, "pb_pbc_int_ecc_nc", 2, ERROR, pb_pbc_int_ecc_nc},
    {PB_PBC_INT_ECC_PACK, "pb_pbc_int_ecc_pack", 2, ERROR, pb_pbc_int_ecc_pack},
    {PB_PBC_INT_ECC_PORT_MON_IN, "pb_pbc_int_ecc_port_mon_in", 2, ERROR, pb_pbc_int_ecc_port_mon},
    {PB_PBC_INT_ECC_PORT_MON_OUT, "pb_pbc_int_ecc_port_mon_out", 2, ERROR, pb_pbc_int_ecc_port_mon},
    {PB_PBC_INT_ECC_RC, "pb_pbc_int_ecc_rc", 2, ERROR, pb_pbc_int_ecc_rc},
    {PB_PBC_INT_ECC_RWR, "pb_pbc_int_ecc_rwr", 2, ERROR, pb_pbc_int_ecc_rwr},
    {PB_PBC_INT_ECC_SCHED, "pb_pbc_int_ecc_sched", 2, ERROR, pb_pbc_int_ecc_sched},
    {PB_PBC_INT_ECC_SIDEBAND, "pb_pbc_int_ecc_sideband", 2, ERROR, pb_pbc_int_ecc_sideband},
    {PB_PBC_INT_ECC_UC, "pb_pbc_int_ecc_uc", 2, ERROR, pb_pbc_int_ecc_uc},
    {PB_PBC_INT_PBUS_VIOLATION_IN, "pb_pbc_int_pbus_violation_in", 24, FATAL, pb_pbc_int_pbus_violation},
    {PB_PBC_INT_PBUS_VIOLATION_OUT, "pb_pbc_int_pbus_violation_out", 24, FATAL, pb_pbc_int_pbus_violation},
    {PB_PBC_INT_ECC_RPL, "pb_pbc_int_rpl", 2, FATAL, pb_pbc_int_rpl},
    {PB_PBC_INT_ECC_WRITE_0, "pb_pbc_int_write_0", 14, FATAL, pb_pbc_int_write},
    {PB_PBC_INT_ECC_WRITE_1, "pb_pbc_int_write_1", 14, FATAL, pb_pbc_int_write},
    {PB_PBC_INT_ECC_WRITE_2, "pb_pbc_int_write_2", 14, FATAL, pb_pbc_int_write},
    {PB_PBC_INT_ECC_WRITE_3, "pb_pbc_int_write_3", 14, FATAL, pb_pbc_int_write},
    {PB_PBC_INT_ECC_WRITE_4, "pb_pbc_int_write_4", 14, FATAL, pb_pbc_int_write},
    {PB_PBC_INT_ECC_WRITE_5, "pb_pbc_int_write_5", 14, FATAL, pb_pbc_int_write},
    {PB_PBC_INT_ECC_WRITE_6, "pb_pbc_int_write_6", 14, FATAL, pb_pbc_int_write},
    {PB_PBC_INT_ECC_WRITE_7, "pb_pbc_int_write_7", 14, FATAL, pb_pbc_int_write},
    {PB_PBC_INT_ECC_WRITE_8, "pb_pbc_int_write_8", 14, FATAL, pb_pbc_int_write},
    {PB_PBC_INT_ECC_WRITE_9, "pb_pbc_int_write_9", 14, FATAL, pb_pbc_int_write},
    {PB_PBC_INT_ECC_WRITE_10, "pb_pbc_int_write_10", 14, FATAL, pb_pbc_int_write},
    {PB_PBC_INT_ECC_WRITE_11, "pb_pbc_int_write_11", 14, FATAL, pb_pbc_int_write},
    {PB_PBC_HBM_INT_ECC_HBM_HT, "pb_pbc_hbm_int_ecc_hbm_ht", 2, ERROR, pb_pbc_hbm_int_ecc_hbm_ht},
    {PB_PBC_HBM_INT_ECC_HBM_MTU, "pb_pbc_hbm_int_ecc_hbm_mtu", 2, ERROR, pb_pbc_hbm_int_ecc_hbm_mtu},
    {PB_PBC_HBM_INT_ECC_HBM_RB, "pb_pbc_hbm_int_ecc_hbm_rb", 4, ERROR, pb_pbc_hbm_int_ecc_hbm_rb},
    {PB_PBC_HBM_INT_ECC_HBM_WB, "pb_pbc_hbm_int_ecc_hbm_wb", 2, ERROR, pb_pbc_hbm_int_ecc_hbm_wb},
    {PB_PBC_HBM_INT_ECC_HBM_AXI_ERR_RSP, "pb_pbc_hbm_int_hbm_axi_err_rsp", 3, FATAL, pb_pbc_hbm_int_hbm_axi_err_rsp},
    {PB_PBC_HBM_INT_ECC_HBM_DROP, "pb_pbc_hbm_int_hbm_drop", 20, INFO, pb_pbc_hbm_int_hbm_drop},
    {PB_PBC_HBM_INT_ECC_HBM_PBUS_VIOLATION_IN, "pb_pbc_hbm_int_hbm_pbus_violation_in", 20, FATAL, pb_pbc_hbm_int_hbm_pbus_violation},
    {PB_PBC_HBM_INT_ECC_HBM_PBUS_VIOLATION_OUT, "pb_pbc_hbm_int_hbm_pbus_violation_out", 20, FATAL, pb_pbc_hbm_int_hbm_pbus_violation},
    {PB_PBC_HBM_INT_ECC_HBM_XOFF, "pb_pbc_hbm_int_hbm_xoff", 1, INFO, pb_pbc_hbm_int_hbm_xoff},
//    {PM_PBM_INT_ECC_COL0, "pm_pbm_int_ecc_col0", 2, ERROR, pm_pbm_int_ecc_col},
//    {PM_PBM_INT_ECC_COL1, "pm_pbm_int_ecc_col1", 2, ERROR, pm_pbm_int_ecc_col},
//    {PM_PBM_INT_ECC_COL2, "pm_pbm_int_ecc_col2", 2, ERROR, pm_pbm_int_ecc_col},
//    {PM_PBM_INT_ECC_COL3, "pm_pbm_int_ecc_col3", 2, ERROR, pm_pbm_int_ecc_col},
//    {PM_PBM_INT_ECC_COL4, "pm_pbm_int_ecc_col4", 2, ERROR, pm_pbm_int_ecc_col},
    {MC0_MCH_INT_MC_INTREG, "mc0_mch_int_mc", 2, ERROR, mc_mch_int_mc},
    {MC1_MCH_INT_MC_INTREG, "mc1_mch_int_mc", 2, ERROR, mc_mch_int_mc},
    {MC2_MCH_INT_MC_INTREG, "mc2_mch_int_mc", 2, ERROR, mc_mch_int_mc},
    {MC3_MCH_INT_MC_INTREG, "mc3_mch_int_mc", 2, ERROR, mc_mch_int_mc},
    {MC4_MCH_INT_MC_INTREG, "mc4_mch_int_mc", 2, ERROR, mc_mch_int_mc},
    {MC5_MCH_INT_MC_INTREG, "mc5_mch_int_mc", 2, ERROR, mc_mch_int_mc},
    {MC6_MCH_INT_MC_INTREG, "mc6_mch_int_mc", 2, ERROR, mc_mch_int_mc},
    {MC7_MCH_INT_MC_INTREG, "mc7_mch_int_mc", 2, ERROR, mc_mch_int_mc},
};

int asic_registers_count = ARRAY_SIZE(capri_registers);
