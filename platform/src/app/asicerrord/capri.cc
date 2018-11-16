#include "asicerrord.h"
#include "nic/sdk/include/sdk/pal.hpp"
#include "nic/sdk/include/sdk/thread.hpp"

CAPRI_INTR_KIND_BEGIN(dppintcredit, 9, DppintcreditMetrics)
    CAPRI_INTR_KIND_FIELD(ptr_credit_ovflow, 0, FATAL)
    CAPRI_INTR_KIND_FIELD(ptr_credit_undflow, 1, FATAL)
    CAPRI_INTR_KIND_FIELD(pkt_credit_ovflow, 2, FATAL)
    CAPRI_INTR_KIND_FIELD(pkt_credit_undflow, 3, FATAL)
    CAPRI_INTR_KIND_FIELD(framer_credit_ovflow, 4, FATAL)
    CAPRI_INTR_KIND_FIELD(framer_credit_undflow, 5, FATAL)
    CAPRI_INTR_KIND_FIELD(framer_hdrfld_vld_ovfl, 6, FATAL)
    CAPRI_INTR_KIND_FIELD(framer_hdrfld_offset_ovfl, 7, FATAL)
    CAPRI_INTR_KIND_FIELD(err_framer_hdrsize_zero_ovfl, 8, FATAL)
CAPRI_INTR_KIND_END(DppintcreditMetrics)

CAPRI_INTR_KIND_BEGIN(dppintfifo, 6, DppintfifoMetrics)
    CAPRI_INTR_KIND_FIELD(phv_ff_overflow, 0, FATAL)
    CAPRI_INTR_KIND_FIELD(ohi_ff_overflow, 1, FATAL)
    CAPRI_INTR_KIND_FIELD(pkt_size_ff_ovflow, 2, FATAL)
    CAPRI_INTR_KIND_FIELD(pkt_size_ff_undflow, 3, FATAL)
    CAPRI_INTR_KIND_FIELD(csum_phv_ff_ovflow, 4, FATAL)
    CAPRI_INTR_KIND_FIELD(csum_phv_ff_undflow, 5, FATAL)
CAPRI_INTR_KIND_END(DppintfifoMetrics)

CAPRI_INTR_KIND_BEGIN(dppintreg1, 17, Dppintreg1Metrics)
    CAPRI_INTR_KIND_FIELD(err_phv_sop_no_eop, 0, ERROR)
    CAPRI_INTR_KIND_FIELD(err_phv_eop_no_sop, 1, ERROR)
    CAPRI_INTR_KIND_FIELD(err_ohi_sop_no_eop, 2, ERROR)
    CAPRI_INTR_KIND_FIELD(err_ohi_eop_no_sop, 3, ERROR)
    CAPRI_INTR_KIND_FIELD(err_framer_credit_overrun, 4, ERROR)
    CAPRI_INTR_KIND_FIELD(err_packets_in_flight_credit_overrun, 5, ERROR)
    CAPRI_INTR_KIND_FIELD(err_null_hdr_vld, 6, ERROR)
    CAPRI_INTR_KIND_FIELD(err_null_hdrfld_vld, 7, ERROR)
    CAPRI_INTR_KIND_FIELD(err_max_pkt_size, 8, ERROR)
    CAPRI_INTR_KIND_FIELD(err_max_active_hdrs, 9, ERROR)
    CAPRI_INTR_KIND_FIELD(err_phv_no_data_reference_ohi, 10, ERROR)
    CAPRI_INTR_KIND_FIELD(err_csum_multiple_hdr, 11, ERROR)
    CAPRI_INTR_KIND_FIELD(err_csum_multiple_hdr_copy, 12, ERROR)
    CAPRI_INTR_KIND_FIELD(err_crc_multiple_hdr, 13, ERROR)
    CAPRI_INTR_KIND_FIELD(err_ptr_fifo_credit_overrun, 14, ERROR)
    CAPRI_INTR_KIND_FIELD(err_clip_max_pkt_size, 15, ERROR)
    CAPRI_INTR_KIND_FIELD(err_min_pkt_size, 16, ERROR)
CAPRI_INTR_KIND_END(Dppintreg1Metrics)

CAPRI_INTR_KIND_BEGIN(dppintreg2, 2, Dppintreg2Metrics)
    CAPRI_INTR_KIND_FIELD(fieldC, 0, ERROR)
    CAPRI_INTR_KIND_FIELD(fieldD, 1, ERROR)
CAPRI_INTR_KIND_END(Dppintreg2Metrics)

CAPRI_INTR_KIND_BEGIN(intspare, 32, IntspareMetrics)
    CAPRI_INTR_KIND_FIELD(spare_0, 0, ERROR)
    CAPRI_INTR_KIND_FIELD(spare_1, 1, ERROR)
    CAPRI_INTR_KIND_FIELD(spare_2, 2, ERROR)
    CAPRI_INTR_KIND_FIELD(spare_3, 3, ERROR)
    CAPRI_INTR_KIND_FIELD(spare_4, 4, ERROR)
    CAPRI_INTR_KIND_FIELD(spare_5, 5, ERROR)
    CAPRI_INTR_KIND_FIELD(spare_6, 6, ERROR)
    CAPRI_INTR_KIND_FIELD(spare_7, 7, ERROR)
    CAPRI_INTR_KIND_FIELD(spare_8, 8, ERROR)
    CAPRI_INTR_KIND_FIELD(spare_9, 9, ERROR)
    CAPRI_INTR_KIND_FIELD(spare_10, 10, ERROR)
    CAPRI_INTR_KIND_FIELD(spare_11, 11, ERROR)
    CAPRI_INTR_KIND_FIELD(spare_12, 12, ERROR)
    CAPRI_INTR_KIND_FIELD(spare_13, 13, ERROR)
    CAPRI_INTR_KIND_FIELD(spare_14, 14, ERROR)
    CAPRI_INTR_KIND_FIELD(spare_15, 15, ERROR)
    CAPRI_INTR_KIND_FIELD(spare_16, 16, ERROR)
    CAPRI_INTR_KIND_FIELD(spare_17, 17, ERROR)
    CAPRI_INTR_KIND_FIELD(spare_18, 18, ERROR)
    CAPRI_INTR_KIND_FIELD(spare_19, 19, ERROR)
    CAPRI_INTR_KIND_FIELD(spare_20, 20, ERROR)
    CAPRI_INTR_KIND_FIELD(spare_21, 21, ERROR)
    CAPRI_INTR_KIND_FIELD(spare_22, 22, ERROR)
    CAPRI_INTR_KIND_FIELD(spare_23, 23, ERROR)
    CAPRI_INTR_KIND_FIELD(spare_24, 24, ERROR)
    CAPRI_INTR_KIND_FIELD(spare_25, 25, ERROR)
    CAPRI_INTR_KIND_FIELD(spare_26, 26, ERROR)
    CAPRI_INTR_KIND_FIELD(spare_27, 27, ERROR)
    CAPRI_INTR_KIND_FIELD(spare_28, 28, ERROR)
    CAPRI_INTR_KIND_FIELD(spare_29, 29, ERROR)
    CAPRI_INTR_KIND_FIELD(spare_30, 30, ERROR)
    CAPRI_INTR_KIND_FIELD(spare_31, 31, ERROR)
CAPRI_INTR_KIND_END(IntspareMetrics)

CAPRI_INTR_KIND_BEGIN(dppintsramsecc, 4, DppintsramseccMetrics)
    CAPRI_INTR_KIND_FIELD(dpp_phv_fifo_uncorrectable, 0, ERROR)
    CAPRI_INTR_KIND_FIELD(dpp_phv_fifo_correctable, 1, ERROR)
    CAPRI_INTR_KIND_FIELD(dpp_ohi_fifo_uncorrectable, 2, ERROR)
    CAPRI_INTR_KIND_FIELD(dpp_ohi_fifo_correctable, 3, ERROR)
CAPRI_INTR_KIND_END(DppintsramseccMetrics)

CAPRI_INTR_KIND_BEGIN(dprintcredit, 4, DprintcreditMetrics)
    CAPRI_INTR_KIND_FIELD(egress_credit_ovflow, 0, FATAL)
    CAPRI_INTR_KIND_FIELD(egress_credit_undflow, 1, FATAL)
    CAPRI_INTR_KIND_FIELD(pktout_credit_ovflow, 2, FATAL)
    CAPRI_INTR_KIND_FIELD(pktout_credit_undflow, 3, FATAL)
CAPRI_INTR_KIND_END(DprintcreditMetrics)

CAPRI_INTR_KIND_BEGIN(dprintfifo, 6, DprintfifoMetrics)
    CAPRI_INTR_KIND_FIELD(phv_ff_ovflow, 0, FATAL)
    CAPRI_INTR_KIND_FIELD(ohi_ff_ovflow, 1, FATAL)
    CAPRI_INTR_KIND_FIELD(pktin_ff_ovflow, 2, FATAL)
    CAPRI_INTR_KIND_FIELD(pktout_ff_undflow, 3, FATAL)
    CAPRI_INTR_KIND_FIELD(csum_ff_ovflow, 4, FATAL)
    CAPRI_INTR_KIND_FIELD(ptr_ff_ovflow, 5, FATAL)
CAPRI_INTR_KIND_END(DprintfifoMetrics)

CAPRI_INTR_KIND_BEGIN(dprintflopfifo, 20, DprintflopfifoMetrics)
    CAPRI_INTR_KIND_FIELD(data_mux_force_bypass_crc_flop_ff_ovflow, 0, FATAL)
    CAPRI_INTR_KIND_FIELD(dpr_crc_info_flop_ff_ovflow, 1, FATAL)
    CAPRI_INTR_KIND_FIELD(dpr_crc_update_info_flop_ff_ovflow, 2, FATAL)
    CAPRI_INTR_KIND_FIELD(dpr_csum_info_flop_ff_ovflow, 3, FATAL)
    CAPRI_INTR_KIND_FIELD(data_mux_force_bypass_csum_flop_ff_ovflow, 4, FATAL)
    CAPRI_INTR_KIND_FIELD(dpr_csum_update_info_flop_ff_ovflow, 5, FATAL)
    CAPRI_INTR_KIND_FIELD(ptr_early_pkt_eop_info_flop_ff_ovflow, 6, FATAL)
    CAPRI_INTR_KIND_FIELD(data_mux_eop_err_flop_ff_ovflow, 7, FATAL)
    CAPRI_INTR_KIND_FIELD(pktin_eop_err_flop_ff_ovflow, 8, FATAL)
    CAPRI_INTR_KIND_FIELD(csum_err_flop_ff_ovflow, 9, FATAL)
    CAPRI_INTR_KIND_FIELD(crc_err_flop_ff_ovflow, 10, FATAL)
    CAPRI_INTR_KIND_FIELD(data_mux_drop_flop_ff_ovflow, 11, FATAL)
    CAPRI_INTR_KIND_FIELD(phv_pkt_data_flop_ff_ovflow, 12, FATAL)
    CAPRI_INTR_KIND_FIELD(pktout_len_cell_flop_ff_ovflow, 13, FATAL)
    CAPRI_INTR_KIND_FIELD(padding_size_flop_ff_ovflow, 14, FATAL)
    CAPRI_INTR_KIND_FIELD(pktin_err_flop_ff_ovflow, 15, FATAL)
    CAPRI_INTR_KIND_FIELD(phv_no_data_flop_ff_ovflow, 16, FATAL)
    CAPRI_INTR_KIND_FIELD(ptr_lookahaed_flop_ff_ovflow, 17, FATAL)
    CAPRI_INTR_KIND_FIELD(eop_vld_flop_ff_ovflow, 18, FATAL)
    CAPRI_INTR_KIND_FIELD(csum_cal_vld_flop_ff_ovflow, 19, FATAL)
CAPRI_INTR_KIND_END(DprintflopfifoMetrics)

CAPRI_INTR_KIND_BEGIN(dprintreg1, 29, Dprintreg1Metrics)
    CAPRI_INTR_KIND_FIELD(err_phv_sop_no_eop, 0, ERROR)
    CAPRI_INTR_KIND_FIELD(err_phv_eop_no_sop, 1, ERROR)
    CAPRI_INTR_KIND_FIELD(err_ohi_sop_no_eop, 2, ERROR)
    CAPRI_INTR_KIND_FIELD(err_ohi_eop_no_sop, 3, ERROR)
    CAPRI_INTR_KIND_FIELD(err_pktin_sop_no_eop, 4, ERROR)
    CAPRI_INTR_KIND_FIELD(err_pktin_eop_no_sop, 5, ERROR)
    CAPRI_INTR_KIND_FIELD(err_csum_offset_gt_pkt_size_4, 6, ERROR)
    CAPRI_INTR_KIND_FIELD(err_csum_offset_gt_pkt_size_3, 7, ERROR)
    CAPRI_INTR_KIND_FIELD(err_csum_offset_gt_pkt_size_2, 8, ERROR)
    CAPRI_INTR_KIND_FIELD(err_csum_offset_gt_pkt_size_1, 9, ERROR)
    CAPRI_INTR_KIND_FIELD(err_csum_offset_gt_pkt_size_0, 10, ERROR)
    CAPRI_INTR_KIND_FIELD(err_csum_phdr_offset_gt_pkt_size_4, 11, ERROR)
    CAPRI_INTR_KIND_FIELD(err_csum_phdr_offset_gt_pkt_size_3, 12, ERROR)
    CAPRI_INTR_KIND_FIELD(err_csum_phdr_offset_gt_pkt_size_2, 13, ERROR)
    CAPRI_INTR_KIND_FIELD(err_csum_phdr_offset_gt_pkt_size_1, 14, ERROR)
    CAPRI_INTR_KIND_FIELD(err_csum_phdr_offset_gt_pkt_size_0, 15, ERROR)
    CAPRI_INTR_KIND_FIELD(err_csum_loc_gt_pkt_size_4, 16, ERROR)
    CAPRI_INTR_KIND_FIELD(err_csum_loc_gt_pkt_size_3, 17, ERROR)
    CAPRI_INTR_KIND_FIELD(err_csum_loc_gt_pkt_size_2, 18, ERROR)
    CAPRI_INTR_KIND_FIELD(err_csum_loc_gt_pkt_size_1, 19, ERROR)
    CAPRI_INTR_KIND_FIELD(err_csum_loc_gt_pkt_size_0, 20, ERROR)
    CAPRI_INTR_KIND_FIELD(err_crc_offset_gt_pkt_size, 21, ERROR)
    CAPRI_INTR_KIND_FIELD(err_crc_loc_gt_pkt_size, 22, ERROR)
    CAPRI_INTR_KIND_FIELD(err_crc_mask_offset_gt_pkt_size, 23, ERROR)
    CAPRI_INTR_KIND_FIELD(err_pkt_eop_early, 24, ERROR)
    CAPRI_INTR_KIND_FIELD(err_ptr_ff_overflow, 25, ERROR)
    CAPRI_INTR_KIND_FIELD(err_csum_ff_overflow, 26, ERROR)
    CAPRI_INTR_KIND_FIELD(err_pktout_ff_overflow, 27, ERROR)
    CAPRI_INTR_KIND_FIELD(err_ptr_from_cfg_overflow, 28, ERROR)
CAPRI_INTR_KIND_END(Dprintreg1Metrics)

CAPRI_INTR_KIND_BEGIN(dprintreg2, 8, Dprintreg2Metrics)
    CAPRI_INTR_KIND_FIELD(fieldC, 0, ERROR)
    CAPRI_INTR_KIND_FIELD(fieldD, 1, ERROR)
    CAPRI_INTR_KIND_FIELD(err_csum_start_gt_end_4, 2, ERROR)
    CAPRI_INTR_KIND_FIELD(err_csum_start_gt_end_3, 3, ERROR)
    CAPRI_INTR_KIND_FIELD(err_csum_start_gt_end_2, 4, ERROR)
    CAPRI_INTR_KIND_FIELD(err_csum_start_gt_end_1, 5, ERROR)
    CAPRI_INTR_KIND_FIELD(err_csum_start_gt_end_0, 6, ERROR)
    CAPRI_INTR_KIND_FIELD(err_crc_start_gt_end, 7, ERROR)
CAPRI_INTR_KIND_END(Dprintreg2Metrics)

CAPRI_INTR_KIND_BEGIN(dprintsramsecc, 12, DprintsramseccMetrics)
    CAPRI_INTR_KIND_FIELD(dpr_pktin_fifo_uncorrectable, 0, ERROR)
    CAPRI_INTR_KIND_FIELD(dpr_pktin_fifo_correctable, 1, ERROR)
    CAPRI_INTR_KIND_FIELD(dpr_csum_fifo_uncorrectable, 2, ERROR)
    CAPRI_INTR_KIND_FIELD(dpr_csum_fifo_correctable, 3, ERROR)
    CAPRI_INTR_KIND_FIELD(dpr_phv_fifo_uncorrectable, 4, ERROR)
    CAPRI_INTR_KIND_FIELD(dpr_phv_fifo_correctable, 5, ERROR)
    CAPRI_INTR_KIND_FIELD(dpr_ohi_fifo_uncorrectable, 6, ERROR)
    CAPRI_INTR_KIND_FIELD(dpr_ohi_fifo_correctable, 7, ERROR)
    CAPRI_INTR_KIND_FIELD(dpr_ptr_fifo_uncorrectable, 8, ERROR)
    CAPRI_INTR_KIND_FIELD(dpr_ptr_fifo_correctable, 9, ERROR)
    CAPRI_INTR_KIND_FIELD(dpr_pktout_fifo_uncorrectable, 10, ERROR)
    CAPRI_INTR_KIND_FIELD(dpr_pktout_fifo_correctable, 11, ERROR)
CAPRI_INTR_KIND_END(DprintsramseccMetrics)

CAPRI_INTR_KIND_BEGIN(ssepicsintbadaddr, 18, SsepicsintbadaddrMetrics)
    CAPRI_INTR_KIND_FIELD(rdreq0_bad_addr, 0, INFO)
    CAPRI_INTR_KIND_FIELD(wrreq0_bad_addr, 1, INFO)
    CAPRI_INTR_KIND_FIELD(rdreq1_bad_addr, 2, INFO)
    CAPRI_INTR_KIND_FIELD(wrreq1_bad_addr, 3, INFO)
    CAPRI_INTR_KIND_FIELD(rdreq2_bad_addr, 4, INFO)
    CAPRI_INTR_KIND_FIELD(wrreq2_bad_addr, 5, INFO)
    CAPRI_INTR_KIND_FIELD(rdreq3_bad_addr, 6, INFO)
    CAPRI_INTR_KIND_FIELD(wrreq3_bad_addr, 7, INFO)
    CAPRI_INTR_KIND_FIELD(rdreq4_bad_addr, 8, INFO)
    CAPRI_INTR_KIND_FIELD(wrreq4_bad_addr, 9, INFO)
    CAPRI_INTR_KIND_FIELD(rdreq5_bad_addr, 10, INFO)
    CAPRI_INTR_KIND_FIELD(wrreq5_bad_addr, 11, INFO)
    CAPRI_INTR_KIND_FIELD(rdreq6_bad_addr, 12, INFO)
    CAPRI_INTR_KIND_FIELD(wrreq6_bad_addr, 13, INFO)
    CAPRI_INTR_KIND_FIELD(rdreq7_bad_addr, 14, INFO)
    CAPRI_INTR_KIND_FIELD(wrreq7_bad_addr, 15, INFO)
    CAPRI_INTR_KIND_FIELD(cpu_bad_addr, 16, INFO)
    CAPRI_INTR_KIND_FIELD(bg_bad_addr, 17, INFO)
CAPRI_INTR_KIND_END(SsepicsintbadaddrMetrics)

CAPRI_INTR_KIND_BEGIN(ssepicsintbg, 16, SsepicsintbgMetrics)
    CAPRI_INTR_KIND_FIELD(unfinished_bg0, 2, INFO)
    CAPRI_INTR_KIND_FIELD(unfinished_bg1, 3, INFO)
    CAPRI_INTR_KIND_FIELD(unfinished_bg2, 4, INFO)
    CAPRI_INTR_KIND_FIELD(unfinished_bg3, 5, INFO)
    CAPRI_INTR_KIND_FIELD(unfinished_bg4, 6, INFO)
    CAPRI_INTR_KIND_FIELD(unfinished_bg5, 7, INFO)
    CAPRI_INTR_KIND_FIELD(unfinished_bg6, 8, INFO)
    CAPRI_INTR_KIND_FIELD(unfinished_bg7, 9, INFO)
    CAPRI_INTR_KIND_FIELD(unfinished_bg8, 10, INFO)
    CAPRI_INTR_KIND_FIELD(unfinished_bg9, 11, INFO)
    CAPRI_INTR_KIND_FIELD(unfinished_bg10, 12, INFO)
    CAPRI_INTR_KIND_FIELD(unfinished_bg11, 13, INFO)
    CAPRI_INTR_KIND_FIELD(unfinished_bg12, 14, INFO)
    CAPRI_INTR_KIND_FIELD(unfinished_bg13, 15, INFO)
    CAPRI_INTR_KIND_FIELD(unfinished_bg14, 16, INFO)
    CAPRI_INTR_KIND_FIELD(unfinished_bg15, 17, INFO)
CAPRI_INTR_KIND_END(SsepicsintbgMetrics)

CAPRI_INTR_KIND_BEGIN(ssepicsintpics, 3, SsepicsintpicsMetrics)
    CAPRI_INTR_KIND_FIELD(uncorrectable_ecc, 0, ERROR)
    CAPRI_INTR_KIND_FIELD(correctable_ecc, 1, ERROR)
    CAPRI_INTR_KIND_FIELD(too_many_rl_sch_error, 2, ERROR)
CAPRI_INTR_KIND_END(SsepicsintpicsMetrics)

CAPRI_INTR_KIND_BEGIN(dbwaintdb, 7, DbwaintdbMetrics)
    CAPRI_INTR_KIND_FIELD(db_cam_conflict, 0, ERROR)
    CAPRI_INTR_KIND_FIELD(db_pid_chk_fail, 1, ERROR)
    CAPRI_INTR_KIND_FIELD(db_qid_overflow, 2, ERROR)
    CAPRI_INTR_KIND_FIELD(host_ring_access_err, 3, ERROR)
    CAPRI_INTR_KIND_FIELD(total_ring_access_err, 4, ERROR)
    CAPRI_INTR_KIND_FIELD(rresp_err, 5, ERROR)
    CAPRI_INTR_KIND_FIELD(bresp_err, 6, ERROR)
CAPRI_INTR_KIND_END(DbwaintdbMetrics)

CAPRI_INTR_KIND_BEGIN(dbwaintlifqstatemap, 3, DbwaintlifqstatemapMetrics)
    CAPRI_INTR_KIND_FIELD(ecc_uncorrectable, 0, ERROR)
    CAPRI_INTR_KIND_FIELD(ecc_correctable, 1, ERROR)
    CAPRI_INTR_KIND_FIELD(qid_invalid, 2, ERROR)
CAPRI_INTR_KIND_END(DbwaintlifqstatemapMetrics)

CAPRI_INTR_KIND_BEGIN(sgeteinterr, 20, SgeteinterrMetrics)
    CAPRI_INTR_KIND_FIELD(miss_sop, 0, FATAL)
    CAPRI_INTR_KIND_FIELD(miss_eop, 1, FATAL)
    CAPRI_INTR_KIND_FIELD(phv_max_size, 2, FATAL)
    CAPRI_INTR_KIND_FIELD(spurious_axi_rsp, 3, FATAL)
    CAPRI_INTR_KIND_FIELD(spurious_tcam_rsp, 4, FATAL)
    CAPRI_INTR_KIND_FIELD(te2mpu_timeout, 5, FATAL)
    CAPRI_INTR_KIND_FIELD(axi_rdrsp_err, 6, FATAL)
    CAPRI_INTR_KIND_FIELD(axi_bad_read, 7, FATAL)
    CAPRI_INTR_KIND_FIELD(tcam_req_idx_fifo, 8, FATAL)
    CAPRI_INTR_KIND_FIELD(tcam_rsp_idx_fifo, 9, FATAL)
    CAPRI_INTR_KIND_FIELD(mpu_req_idx_fifo, 10, FATAL)
    CAPRI_INTR_KIND_FIELD(axi_req_idx_fifo, 11, FATAL)
    CAPRI_INTR_KIND_FIELD(proc_tbl_vld_wo_proc, 12, FATAL)
    CAPRI_INTR_KIND_FIELD(pend_wo_wb, 13, FATAL)
    CAPRI_INTR_KIND_FIELD(pend1_wo_pend0, 14, FATAL)
    CAPRI_INTR_KIND_FIELD(both_pend_down, 15, FATAL)
    CAPRI_INTR_KIND_FIELD(pend_wo_proc_down, 16, FATAL)
    CAPRI_INTR_KIND_FIELD(both_pend_went_up, 17, FATAL)
    CAPRI_INTR_KIND_FIELD(loaded_but_no_proc, 18, FATAL)
    CAPRI_INTR_KIND_FIELD(loaded_but_no_proc_tbl_vld, 19, FATAL)
CAPRI_INTR_KIND_END(SgeteinterrMetrics)

CAPRI_INTR_KIND_BEGIN(sgeteintinfo, 17, SgeteintinfoMetrics)
    CAPRI_INTR_KIND_FIELD(profile_cam_hit0, 0, INFO)
    CAPRI_INTR_KIND_FIELD(profile_cam_hit1, 1, INFO)
    CAPRI_INTR_KIND_FIELD(profile_cam_hit2, 2, INFO)
    CAPRI_INTR_KIND_FIELD(profile_cam_hit3, 3, INFO)
    CAPRI_INTR_KIND_FIELD(profile_cam_hit4, 4, INFO)
    CAPRI_INTR_KIND_FIELD(profile_cam_hit5, 5, INFO)
    CAPRI_INTR_KIND_FIELD(profile_cam_hit6, 6, INFO)
    CAPRI_INTR_KIND_FIELD(profile_cam_hit7, 7, INFO)
    CAPRI_INTR_KIND_FIELD(profile_cam_hit8, 8, INFO)
    CAPRI_INTR_KIND_FIELD(profile_cam_hit9, 9, INFO)
    CAPRI_INTR_KIND_FIELD(profile_cam_hit10, 10, INFO)
    CAPRI_INTR_KIND_FIELD(profile_cam_hit11, 11, INFO)
    CAPRI_INTR_KIND_FIELD(profile_cam_hit12, 12, INFO)
    CAPRI_INTR_KIND_FIELD(profile_cam_hit13, 13, INFO)
    CAPRI_INTR_KIND_FIELD(profile_cam_hit14, 14, INFO)
    CAPRI_INTR_KIND_FIELD(profile_cam_hit15, 15, INFO)
    CAPRI_INTR_KIND_FIELD(profile_cam_miss, 16, INFO)
CAPRI_INTR_KIND_END(SgeteintinfoMetrics)

CAPRI_INTR_KIND_BEGIN(sgempuinterr, 27, SgempuinterrMetrics)
    CAPRI_INTR_KIND_FIELD(results_mismatch, 0, ERROR)
    CAPRI_INTR_KIND_FIELD(sdp_mem_uncorrectable, 1, ERROR)
    CAPRI_INTR_KIND_FIELD(sdp_mem_correctable, 2, ERROR)
    CAPRI_INTR_KIND_FIELD(illegal_op_0, 3, ERROR)
    CAPRI_INTR_KIND_FIELD(illegal_op_1, 4, ERROR)
    CAPRI_INTR_KIND_FIELD(illegal_op_2, 5, ERROR)
    CAPRI_INTR_KIND_FIELD(illegal_op_3, 6, ERROR)
    CAPRI_INTR_KIND_FIELD(max_inst_0, 7, ERROR)
    CAPRI_INTR_KIND_FIELD(max_inst_1, 8, ERROR)
    CAPRI_INTR_KIND_FIELD(max_inst_2, 9, ERROR)
    CAPRI_INTR_KIND_FIELD(max_inst_3, 10, ERROR)
    CAPRI_INTR_KIND_FIELD(phvwr_0, 11, ERROR)
    CAPRI_INTR_KIND_FIELD(phvwr_1, 12, ERROR)
    CAPRI_INTR_KIND_FIELD(phvwr_2, 13, ERROR)
    CAPRI_INTR_KIND_FIELD(phvwr_3, 14, ERROR)
    CAPRI_INTR_KIND_FIELD(write_err_0, 15, ERROR)
    CAPRI_INTR_KIND_FIELD(write_err_1, 16, ERROR)
    CAPRI_INTR_KIND_FIELD(write_err_2, 17, ERROR)
    CAPRI_INTR_KIND_FIELD(write_err_3, 18, ERROR)
    CAPRI_INTR_KIND_FIELD(cache_axi_0, 19, ERROR)
    CAPRI_INTR_KIND_FIELD(cache_axi_1, 20, ERROR)
    CAPRI_INTR_KIND_FIELD(cache_axi_2, 21, ERROR)
    CAPRI_INTR_KIND_FIELD(cache_axi_3, 22, ERROR)
    CAPRI_INTR_KIND_FIELD(cache_parity_0, 23, ERROR)
    CAPRI_INTR_KIND_FIELD(cache_parity_1, 24, ERROR)
    CAPRI_INTR_KIND_FIELD(cache_parity_2, 25, ERROR)
    CAPRI_INTR_KIND_FIELD(cache_parity_3, 26, ERROR)
CAPRI_INTR_KIND_END(SgempuinterrMetrics)

CAPRI_INTR_KIND_BEGIN(sgempuintinfo, 8, SgempuintinfoMetrics)
    CAPRI_INTR_KIND_FIELD(trace_full_0, 0, INFO)
    CAPRI_INTR_KIND_FIELD(trace_full_1, 1, INFO)
    CAPRI_INTR_KIND_FIELD(trace_full_2, 2, INFO)
    CAPRI_INTR_KIND_FIELD(trace_full_3, 3, INFO)
    CAPRI_INTR_KIND_FIELD(mpu_stop_0, 4, INFO)
    CAPRI_INTR_KIND_FIELD(mpu_stop_1, 5, INFO)
    CAPRI_INTR_KIND_FIELD(mpu_stop_2, 6, INFO)
    CAPRI_INTR_KIND_FIELD(mpu_stop_3, 7, INFO)
CAPRI_INTR_KIND_END(SgempuintinfoMetrics)

CAPRI_INTR_KIND_BEGIN(mdhensintaxierr, 2, MdhensintaxierrMetrics)
    CAPRI_INTR_KIND_FIELD(wrsp_err, 0, FATAL)
    CAPRI_INTR_KIND_FIELD(rrsp_err, 1, FATAL)
CAPRI_INTR_KIND_END(MdhensintaxierrMetrics)

CAPRI_INTR_KIND_BEGIN(mdhensintecc, 2, MdhensinteccMetrics)
    CAPRI_INTR_KIND_FIELD(correctable_err, 0, ERROR)
    CAPRI_INTR_KIND_FIELD(uncorrectable_err, 1, ERROR)
CAPRI_INTR_KIND_END(MdhensinteccMetrics)

CAPRI_INTR_KIND_BEGIN(mdhensintipcore, 8, MdhensintipcoreMetrics)
    CAPRI_INTR_KIND_FIELD(xts_enc, 0, UNKNOWN)
    CAPRI_INTR_KIND_FIELD(xts, 1, UNKNOWN)
    CAPRI_INTR_KIND_FIELD(gcm0, 2, UNKNOWN)
    CAPRI_INTR_KIND_FIELD(gcm1, 3, UNKNOWN)
    CAPRI_INTR_KIND_FIELD(drbg, 4, UNKNOWN)
    CAPRI_INTR_KIND_FIELD(pk, 5, UNKNOWN)
    CAPRI_INTR_KIND_FIELD(cp, 6, UNKNOWN)
    CAPRI_INTR_KIND_FIELD(dc, 7, UNKNOWN)
CAPRI_INTR_KIND_END(MdhensintipcoreMetrics)

CAPRI_INTR_KIND_BEGIN(mpmpnsintcrypto, 8, MpmpnsintcryptoMetrics)
    CAPRI_INTR_KIND_FIELD(mpp0, 0, UNKNOWN)
    CAPRI_INTR_KIND_FIELD(mpp1, 1, UNKNOWN)
    CAPRI_INTR_KIND_FIELD(mpp2, 2, UNKNOWN)
    CAPRI_INTR_KIND_FIELD(mpp3, 3, UNKNOWN)
    CAPRI_INTR_KIND_FIELD(mpp4, 4, UNKNOWN)
    CAPRI_INTR_KIND_FIELD(mpp5, 5, UNKNOWN)
    CAPRI_INTR_KIND_FIELD(mpp6, 6, UNKNOWN)
    CAPRI_INTR_KIND_FIELD(mpp7, 7, UNKNOWN)
CAPRI_INTR_KIND_END(MpmpnsintcryptoMetrics)

CAPRI_INTR_KIND_BEGIN(pbpbcintcreditunderflow, 2, PbpbcintcreditunderflowMetrics)
    CAPRI_INTR_KIND_FIELD(port_10, 0, FATAL)
    CAPRI_INTR_KIND_FIELD(port_11, 1, FATAL)
CAPRI_INTR_KIND_END(PbpbcintcreditunderflowMetrics)

CAPRI_INTR_KIND_BEGIN(inteccdesc, 2, InteccdescMetrics)
    CAPRI_INTR_KIND_FIELD(uncorrectable, 0, ERROR)
    CAPRI_INTR_KIND_FIELD(correctable, 1, ERROR)
CAPRI_INTR_KIND_END(InteccdescMetrics)

CAPRI_INTR_KIND_BEGIN(pbpbcintpbusviolation, 24, PbpbcintpbusviolationMetrics)
    CAPRI_INTR_KIND_FIELD(sop_sop_in_0, 0, FATAL)
    CAPRI_INTR_KIND_FIELD(sop_sop_in_1, 1, FATAL)
    CAPRI_INTR_KIND_FIELD(sop_sop_in_2, 2, FATAL)
    CAPRI_INTR_KIND_FIELD(sop_sop_in_3, 3, FATAL)
    CAPRI_INTR_KIND_FIELD(sop_sop_in_4, 4, FATAL)
    CAPRI_INTR_KIND_FIELD(sop_sop_in_5, 5, FATAL)
    CAPRI_INTR_KIND_FIELD(sop_sop_in_6, 6, FATAL)
    CAPRI_INTR_KIND_FIELD(sop_sop_in_7, 7, FATAL)
    CAPRI_INTR_KIND_FIELD(sop_sop_in_8, 8, FATAL)
    CAPRI_INTR_KIND_FIELD(sop_sop_in_9, 9, FATAL)
    CAPRI_INTR_KIND_FIELD(sop_sop_in_10, 10, FATAL)
    CAPRI_INTR_KIND_FIELD(sop_sop_in_11, 11, FATAL)
    CAPRI_INTR_KIND_FIELD(eop_eop_in_0, 12, FATAL)
    CAPRI_INTR_KIND_FIELD(eop_eop_in_1, 13, FATAL)
    CAPRI_INTR_KIND_FIELD(eop_eop_in_2, 14, FATAL)
    CAPRI_INTR_KIND_FIELD(eop_eop_in_3, 15, FATAL)
    CAPRI_INTR_KIND_FIELD(eop_eop_in_4, 16, FATAL)
    CAPRI_INTR_KIND_FIELD(eop_eop_in_5, 17, FATAL)
    CAPRI_INTR_KIND_FIELD(eop_eop_in_6, 18, FATAL)
    CAPRI_INTR_KIND_FIELD(eop_eop_in_7, 19, FATAL)
    CAPRI_INTR_KIND_FIELD(eop_eop_in_8, 20, FATAL)
    CAPRI_INTR_KIND_FIELD(eop_eop_in_9, 21, FATAL)
    CAPRI_INTR_KIND_FIELD(eop_eop_in_10, 22, FATAL)
    CAPRI_INTR_KIND_FIELD(eop_eop_in_11, 23, FATAL)
CAPRI_INTR_KIND_END(PbpbcintpbusviolationMetrics)

CAPRI_INTR_KIND_BEGIN(pbpbcintrpl, 2, PbpbcintrplMetrics)
    CAPRI_INTR_KIND_FIELD(memory_error, 0, FATAL)
    CAPRI_INTR_KIND_FIELD(zero_last_error, 1, FATAL)
CAPRI_INTR_KIND_END(PbpbcintrplMetrics)

CAPRI_INTR_KIND_BEGIN(pbpbcintwrite, 14, PbpbcintwriteMetrics)
    CAPRI_INTR_KIND_FIELD(out_of_cells, 0, FATAL)
    CAPRI_INTR_KIND_FIELD(out_of_credit, 1, FATAL)
    CAPRI_INTR_KIND_FIELD(port_disabled, 2, FATAL)
    CAPRI_INTR_KIND_FIELD(truncation, 3, FATAL)
    CAPRI_INTR_KIND_FIELD(intrinsic_drop, 4, FATAL)
    CAPRI_INTR_KIND_FIELD(out_of_cells1, 5, FATAL)
    CAPRI_INTR_KIND_FIELD(enq_err, 6, FATAL)
    CAPRI_INTR_KIND_FIELD(tail_drop_cpu, 7, FATAL)
    CAPRI_INTR_KIND_FIELD(tail_drop_span, 8, FATAL)
    CAPRI_INTR_KIND_FIELD(min_size_viol, 9, FATAL)
    CAPRI_INTR_KIND_FIELD(port_range, 10, FATAL)
    CAPRI_INTR_KIND_FIELD(credit_growth_error, 11, FATAL)
    CAPRI_INTR_KIND_FIELD(oq_range, 12, FATAL)
    CAPRI_INTR_KIND_FIELD(xoff_timeout, 13, FATAL)
CAPRI_INTR_KIND_END(PbpbcintwriteMetrics)

CAPRI_INTR_KIND_BEGIN(pbpbchbmintecchbmrb, 4, PbpbchbmintecchbmrbMetrics)
    CAPRI_INTR_KIND_FIELD(rb_uncorrectable, 0, ERROR)
    CAPRI_INTR_KIND_FIELD(rb_correctable, 1, ERROR)
    CAPRI_INTR_KIND_FIELD(cdt_uncorrectable, 2, ERROR)
    CAPRI_INTR_KIND_FIELD(cdt_correctable, 3, ERROR)
CAPRI_INTR_KIND_END(PbpbchbmintecchbmrbMetrics)

CAPRI_INTR_KIND_BEGIN(pbpbchbminthbmaxierrrsp, 3, PbpbchbminthbmaxierrrspMetrics)
    CAPRI_INTR_KIND_FIELD(ctrl, 0, FATAL)
    CAPRI_INTR_KIND_FIELD(pyld, 1, FATAL)
    CAPRI_INTR_KIND_FIELD(r2a, 2, FATAL)
CAPRI_INTR_KIND_END(PbpbchbminthbmaxierrrspMetrics)

CAPRI_INTR_KIND_BEGIN(pbpbchbminthbmdrop, 20, PbpbchbminthbmdropMetrics)
    CAPRI_INTR_KIND_FIELD(occupancy_0, 0, INFO)
    CAPRI_INTR_KIND_FIELD(occupancy_1, 1, INFO)
    CAPRI_INTR_KIND_FIELD(occupancy_2, 2, INFO)
    CAPRI_INTR_KIND_FIELD(occupancy_3, 3, INFO)
    CAPRI_INTR_KIND_FIELD(occupancy_4, 4, INFO)
    CAPRI_INTR_KIND_FIELD(occupancy_5, 5, INFO)
    CAPRI_INTR_KIND_FIELD(occupancy_6, 6, INFO)
    CAPRI_INTR_KIND_FIELD(occupancy_7, 7, INFO)
    CAPRI_INTR_KIND_FIELD(occupancy_8, 8, INFO)
    CAPRI_INTR_KIND_FIELD(occupancy_9, 9, INFO)
    CAPRI_INTR_KIND_FIELD(ctrl_full_0, 10, INFO)
    CAPRI_INTR_KIND_FIELD(ctrl_full_1, 11, INFO)
    CAPRI_INTR_KIND_FIELD(ctrl_full_2, 12, INFO)
    CAPRI_INTR_KIND_FIELD(ctrl_full_3, 13, INFO)
    CAPRI_INTR_KIND_FIELD(ctrl_full_4, 14, INFO)
    CAPRI_INTR_KIND_FIELD(ctrl_full_5, 15, INFO)
    CAPRI_INTR_KIND_FIELD(ctrl_full_6, 16, INFO)
    CAPRI_INTR_KIND_FIELD(ctrl_full_7, 17, INFO)
    CAPRI_INTR_KIND_FIELD(ctrl_full_8, 18, INFO)
    CAPRI_INTR_KIND_FIELD(ctrl_full_9, 19, INFO)
CAPRI_INTR_KIND_END(PbpbchbminthbmdropMetrics)

CAPRI_INTR_KIND_BEGIN(pbpbchbminthbmpbusviolation, 20, PbpbchbminthbmpbusviolationMetrics)
    CAPRI_INTR_KIND_FIELD(sop_sop_in_0, 0, FATAL)
    CAPRI_INTR_KIND_FIELD(sop_sop_in_1, 1, FATAL)
    CAPRI_INTR_KIND_FIELD(sop_sop_in_2, 2, FATAL)
    CAPRI_INTR_KIND_FIELD(sop_sop_in_3, 3, FATAL)
    CAPRI_INTR_KIND_FIELD(sop_sop_in_4, 4, FATAL)
    CAPRI_INTR_KIND_FIELD(sop_sop_in_5, 5, FATAL)
    CAPRI_INTR_KIND_FIELD(sop_sop_in_6, 6, FATAL)
    CAPRI_INTR_KIND_FIELD(sop_sop_in_7, 7, FATAL)
    CAPRI_INTR_KIND_FIELD(sop_sop_in_8, 8, FATAL)
    CAPRI_INTR_KIND_FIELD(sop_sop_in_9, 9, FATAL)
    CAPRI_INTR_KIND_FIELD(eop_eop_in_0, 10, FATAL)
    CAPRI_INTR_KIND_FIELD(eop_eop_in_1, 11, FATAL)
    CAPRI_INTR_KIND_FIELD(eop_eop_in_2, 12, FATAL)
    CAPRI_INTR_KIND_FIELD(eop_eop_in_3, 13, FATAL)
    CAPRI_INTR_KIND_FIELD(eop_eop_in_4, 14, FATAL)
    CAPRI_INTR_KIND_FIELD(eop_eop_in_5, 15, FATAL)
    CAPRI_INTR_KIND_FIELD(eop_eop_in_6, 16, FATAL)
    CAPRI_INTR_KIND_FIELD(eop_eop_in_7, 17, FATAL)
    CAPRI_INTR_KIND_FIELD(eop_eop_in_8, 18, FATAL)
    CAPRI_INTR_KIND_FIELD(eop_eop_in_9, 19, FATAL)
CAPRI_INTR_KIND_END(PbpbchbminthbmpbusviolationMetrics)

CAPRI_INTR_KIND_BEGIN(pbpbchbminthbmxoff, 1, PbpbchbminthbmxoffMetrics)
    CAPRI_INTR_KIND_FIELD(timeout, 0, INFO)
CAPRI_INTR_KIND_END(PbpbchbminthbmxoffMetrics)

CAPRI_INTR_KIND_BEGIN(mcmchintmc, 2, McmchintmcMetrics)
    CAPRI_INTR_KIND_FIELD(ecc_1bit_thres_ps1, 0, ERROR)
    CAPRI_INTR_KIND_FIELD(ecc_1bit_thres_ps0, 1, ERROR)
CAPRI_INTR_KIND_END(McmchintmcMetrics)

void poll_capri_intr() {
    CAPRI_INTR_READ(dppintcreditmetrics, 0, DPP0_INT_CREDIT);
    CAPRI_INTR_READ(dppintcreditmetrics, 1, DPP1_INT_CREDIT);
    CAPRI_INTR_READ(dppintfifometrics, 0, DPP0_INT_FIFO);
    CAPRI_INTR_READ(dppintfifometrics, 1, DPP1_INT_FIFO);
    CAPRI_INTR_READ(dppintreg1metrics, 0, DPP0_INT_REG1);
    CAPRI_INTR_READ(dppintreg2metrics, 0, DPP0_INT_REG2);
    CAPRI_INTR_READ(dppintreg1metrics, 1, DPP0_INT_REG1);
    CAPRI_INTR_READ(dppintreg2metrics, 1, DPP0_INT_REG2);
    CAPRI_INTR_READ(intsparemetrics, 0, DPP0_INT_SPARE);
    CAPRI_INTR_READ(intsparemetrics, 1, DPP1_INT_SPARE);
    CAPRI_INTR_READ(dppintsramseccmetrics, 0, DPP0_INT_SRAMS_ECC);
    CAPRI_INTR_READ(dppintsramseccmetrics, 1, DPP1_INT_SRAMS_ECC);
    CAPRI_INTR_READ(dprintcreditmetrics, 0, DPR0_INT_CREDIT);
    CAPRI_INTR_READ(dprintcreditmetrics, 1, DPR1_INT_CREDIT);
    CAPRI_INTR_READ(dprintfifometrics, 0, DPR0_INT_FIFO);
    CAPRI_INTR_READ(dprintfifometrics, 1, DPR1_INT_FIFO);
    CAPRI_INTR_READ(dprintflopfifometrics, 0, DPR0_INT_FLOP_FIFO);
    CAPRI_INTR_READ(dprintflopfifometrics, 1, DPR1_INT_FLOP_FIFO);
    CAPRI_INTR_READ(dprintreg1metrics, 0, DPR0_INT_REG1);
    CAPRI_INTR_READ(dprintreg2metrics, 0, DPR0_INT_REG2);
    CAPRI_INTR_READ(dprintreg1metrics, 1, DPR1_INT_REG1);
    CAPRI_INTR_READ(dprintreg2metrics, 1, DPR1_INT_REG2);
    CAPRI_INTR_READ(intsparemetrics, 2, DPR0_INT_SPARE);
    CAPRI_INTR_READ(intsparemetrics, 3, DPR1_INT_SPARE);
    CAPRI_INTR_READ(dprintsramseccmetrics, 0, DPR0_INT_SRAMS_ECC);
    CAPRI_INTR_READ(dprintsramseccmetrics, 1, DPR1_INT_SRAMS_ECC);
    CAPRI_INTR_READ(ssepicsintbadaddrmetrics, 0, SSE_PICS_INT_BADADDR);
    CAPRI_INTR_READ(ssepicsintbgmetrics, 0, SSE_PICS_INT_BG);
    CAPRI_INTR_READ(ssepicsintpicsmetrics, 0, SSE_PICS_INT_PICS);
//    CAPRI_INTR_READ(dbwaintdbmetrics, 0, DB_WA_INT_DB);
//    CAPRI_INTR_READ(dbwaintlifqstatemapmetrics, 0, DB_WA_INT_LIF_QSTATE_MAP);
    CAPRI_INTR_READ(sgeteinterrmetrics, 0, SGE_TE0_INT_ERR);
    CAPRI_INTR_READ(sgeteinterrmetrics, 1, SGE_TE1_INT_ERR);
    CAPRI_INTR_READ(sgeteinterrmetrics, 2, SGE_TE2_INT_ERR);
    CAPRI_INTR_READ(sgeteinterrmetrics, 3, SGE_TE3_INT_ERR);
    CAPRI_INTR_READ(sgeteinterrmetrics, 4, SGE_TE4_INT_ERR);
    CAPRI_INTR_READ(sgeteinterrmetrics, 5, SGE_TE5_INT_ERR);
    CAPRI_INTR_READ(sgeteintinfometrics, 0, SGE_TE0_INT_INFO);
    CAPRI_INTR_READ(sgeteintinfometrics, 1, SGE_TE1_INT_INFO);
    CAPRI_INTR_READ(sgeteintinfometrics, 2, SGE_TE2_INT_INFO);
    CAPRI_INTR_READ(sgeteintinfometrics, 3, SGE_TE3_INT_INFO);
    CAPRI_INTR_READ(sgeteintinfometrics, 4, SGE_TE4_INT_INFO);
    CAPRI_INTR_READ(sgeteintinfometrics, 5, SGE_TE5_INT_INFO);
    CAPRI_INTR_READ(sgempuinterrmetrics, 0, SGE_MPU0_INT_ERR);
    CAPRI_INTR_READ(sgempuinterrmetrics, 1, SGE_MPU1_INT_ERR);
    CAPRI_INTR_READ(sgempuinterrmetrics, 2, SGE_MPU2_INT_ERR);
    CAPRI_INTR_READ(sgempuinterrmetrics, 3, SGE_MPU3_INT_ERR);
    CAPRI_INTR_READ(sgempuinterrmetrics, 4, SGE_MPU4_INT_ERR);
    CAPRI_INTR_READ(sgempuinterrmetrics, 5, SGE_MPU5_INT_ERR);
    CAPRI_INTR_READ(sgempuintinfometrics, 0, SGE_MPU0_INT_INFO);
    CAPRI_INTR_READ(sgempuintinfometrics, 1, SGE_MPU1_INT_INFO);
    CAPRI_INTR_READ(sgempuintinfometrics, 2, SGE_MPU2_INT_INFO);
    CAPRI_INTR_READ(sgempuintinfometrics, 3, SGE_MPU3_INT_INFO);
    CAPRI_INTR_READ(sgempuintinfometrics, 4, SGE_MPU4_INT_INFO);
    CAPRI_INTR_READ(sgempuintinfometrics, 5, SGE_MPU5_INT_INFO);
    CAPRI_INTR_READ(mdhensintaxierrmetrics, 0, MD_HENS_INT_AXI_ERR);
    CAPRI_INTR_READ(mdhensinteccmetrics, 0, MD_HENS_INT_DRBG_CRYPTORAM_ECC);
    CAPRI_INTR_READ(mdhensinteccmetrics, 1, MD_HENS_INT_DRBG_INTRAM_ECC);
    CAPRI_INTR_READ(mdhensintipcoremetrics, 0, MD_HENS_INT_IPCORE);
    CAPRI_INTR_READ(mdhensinteccmetrics, 2, MD_HENS_INT_PK_ECC);
    CAPRI_INTR_READ(mpmpnsintcryptometrics, 0, MP_MPNS_INT_CRYPTO);
    CAPRI_INTR_READ(pbpbcintcreditunderflowmetrics, 0, PB_PBC_INT_CREDIT_UNDERFLOW);
    CAPRI_INTR_READ(inteccdescmetrics, 0, PB_PBC_INT_ECC_DESC_0);
    CAPRI_INTR_READ(inteccdescmetrics, 1, PB_PBC_INT_ECC_DESC_1);
    CAPRI_INTR_READ(inteccdescmetrics, 2, PB_PBC_INT_ECC_FC_0);
    CAPRI_INTR_READ(inteccdescmetrics, 3, PB_PBC_INT_ECC_FC_1);
    CAPRI_INTR_READ(inteccdescmetrics, 4, PB_PBC_INT_ECC_LL_0);
    CAPRI_INTR_READ(inteccdescmetrics, 5, PB_PBC_INT_ECC_LL_1);
    CAPRI_INTR_READ(inteccdescmetrics, 6, PB_PBC_INT_ECC_NC);
    CAPRI_INTR_READ(inteccdescmetrics, 7, PB_PBC_INT_ECC_PACK);
    CAPRI_INTR_READ(inteccdescmetrics, 8, PB_PBC_INT_ECC_PORT_MON_IN);
    CAPRI_INTR_READ(inteccdescmetrics, 9, PB_PBC_INT_ECC_PORT_MON_OUT);
    CAPRI_INTR_READ(inteccdescmetrics, 10, PB_PBC_INT_ECC_RC);
    CAPRI_INTR_READ(inteccdescmetrics, 11, PB_PBC_INT_ECC_RWR);
    CAPRI_INTR_READ(inteccdescmetrics, 12, PB_PBC_INT_ECC_SCHED);
    CAPRI_INTR_READ(inteccdescmetrics, 13, PB_PBC_INT_ECC_SIDEBAND);
    CAPRI_INTR_READ(inteccdescmetrics, 14, PB_PBC_INT_ECC_UC);
    CAPRI_INTR_READ(pbpbcintpbusviolationmetrics, 0, PB_PBC_INT_PBUS_VIOLATION_IN);
    CAPRI_INTR_READ(pbpbcintpbusviolationmetrics, 1, PB_PBC_INT_PBUS_VIOLATION_OUT);
    CAPRI_INTR_READ(pbpbcintrplmetrics, 0, PB_PBC_INT_ECC_RPL);
    CAPRI_INTR_READ(pbpbcintwritemetrics, 0, PB_PBC_INT_ECC_WRITE_0);
    CAPRI_INTR_READ(pbpbcintwritemetrics, 1, PB_PBC_INT_ECC_WRITE_1);
    CAPRI_INTR_READ(pbpbcintwritemetrics, 2, PB_PBC_INT_ECC_WRITE_2);
    CAPRI_INTR_READ(pbpbcintwritemetrics, 3, PB_PBC_INT_ECC_WRITE_3);
    CAPRI_INTR_READ(pbpbcintwritemetrics, 4, PB_PBC_INT_ECC_WRITE_4);
    CAPRI_INTR_READ(pbpbcintwritemetrics, 5, PB_PBC_INT_ECC_WRITE_5);
    CAPRI_INTR_READ(pbpbcintwritemetrics, 6, PB_PBC_INT_ECC_WRITE_6);
    CAPRI_INTR_READ(pbpbcintwritemetrics, 7, PB_PBC_INT_ECC_WRITE_7);
    CAPRI_INTR_READ(pbpbcintwritemetrics, 8, PB_PBC_INT_ECC_WRITE_8);
    CAPRI_INTR_READ(pbpbcintwritemetrics, 9, PB_PBC_INT_ECC_WRITE_9);
    CAPRI_INTR_READ(pbpbcintwritemetrics, 10, PB_PBC_INT_ECC_WRITE_10);
    CAPRI_INTR_READ(pbpbcintwritemetrics, 11, PB_PBC_INT_ECC_WRITE_11);
    CAPRI_INTR_READ(inteccdescmetrics, 15, PB_PBC_HBM_INT_ECC_HBM_HT);
    CAPRI_INTR_READ(inteccdescmetrics, 16, PB_PBC_HBM_INT_ECC_HBM_MTU);
    CAPRI_INTR_READ(pbpbchbmintecchbmrbmetrics, 0, PB_PBC_HBM_INT_ECC_HBM_RB);
    CAPRI_INTR_READ(inteccdescmetrics, 17, PB_PBC_HBM_INT_ECC_HBM_WB);
    CAPRI_INTR_READ(pbpbchbminthbmaxierrrspmetrics, 0, PB_PBC_HBM_INT_ECC_HBM_AXI_ERR_RSP);
    CAPRI_INTR_READ(pbpbchbminthbmdropmetrics, 0, PB_PBC_HBM_INT_ECC_HBM_DROP);
    CAPRI_INTR_READ(pbpbchbminthbmpbusviolationmetrics, 0, PB_PBC_HBM_INT_ECC_HBM_PBUS_VIOLATION_IN);
    CAPRI_INTR_READ(pbpbchbminthbmpbusviolationmetrics, 1, PB_PBC_HBM_INT_ECC_HBM_PBUS_VIOLATION_OUT);
    CAPRI_INTR_READ(pbpbchbminthbmxoffmetrics, 0, PB_PBC_HBM_INT_ECC_HBM_XOFF);
//    CAPRI_INTR_READ(inteccdescmetrics, 18, PM_PBM_INT_ECC_COL0);
//    CAPRI_INTR_READ(inteccdescmetrics, 19, PM_PBM_INT_ECC_COL1);
//    CAPRI_INTR_READ(inteccdescmetrics, 20, PM_PBM_INT_ECC_COL2);
//    CAPRI_INTR_READ(inteccdescmetrics, 21, PM_PBM_INT_ECC_COL3);
//    CAPRI_INTR_READ(inteccdescmetrics, 22, PM_PBM_INT_ECC_COL4);
//    CAPRI_INTR_READ(inteccdescmetrics, 23, PM_PBM_INT_ECC_COL5);
    CAPRI_INTR_READ(mcmchintmcmetrics, 0, MC0_MCH_INT_MC_INTREG);
    CAPRI_INTR_READ(mcmchintmcmetrics, 1, MC1_MCH_INT_MC_INTREG);
    CAPRI_INTR_READ(mcmchintmcmetrics, 2, MC2_MCH_INT_MC_INTREG);
    CAPRI_INTR_READ(mcmchintmcmetrics, 3, MC3_MCH_INT_MC_INTREG);
    CAPRI_INTR_READ(mcmchintmcmetrics, 4, MC4_MCH_INT_MC_INTREG);
    CAPRI_INTR_READ(mcmchintmcmetrics, 5, MC5_MCH_INT_MC_INTREG);
    CAPRI_INTR_READ(mcmchintmcmetrics, 6, MC6_MCH_INT_MC_INTREG);
    CAPRI_INTR_READ(mcmchintmcmetrics, 7, MC7_MCH_INT_MC_INTREG);
    FLUSH();
}

void clear_capri_intr() {
    CAPRI_INTR_CLEAR(dppintcreditmetrics, 0, DPP0_INT_CREDIT);
    CAPRI_INTR_CLEAR(dppintcreditmetrics, 1, DPP1_INT_CREDIT);
    CAPRI_INTR_CLEAR(dppintfifometrics, 0, DPP0_INT_FIFO);
    CAPRI_INTR_CLEAR(dppintfifometrics, 1, DPP1_INT_FIFO);
    CAPRI_INTR_CLEAR(dppintreg1metrics, 0, DPP0_INT_REG1);
    CAPRI_INTR_CLEAR(dppintreg2metrics, 0, DPP0_INT_REG2);
    CAPRI_INTR_CLEAR(dppintreg1metrics, 1, DPP0_INT_REG1);
    CAPRI_INTR_CLEAR(dppintreg2metrics, 1, DPP0_INT_REG2);
    CAPRI_INTR_CLEAR(intsparemetrics, 0, DPP0_INT_SPARE);
    CAPRI_INTR_CLEAR(intsparemetrics, 1, DPP1_INT_SPARE);
    CAPRI_INTR_CLEAR(dppintsramseccmetrics, 0, DPP0_INT_SRAMS_ECC);
    CAPRI_INTR_CLEAR(dppintsramseccmetrics, 1, DPP1_INT_SRAMS_ECC);
    CAPRI_INTR_CLEAR(dprintcreditmetrics, 0, DPR0_INT_CREDIT);
    CAPRI_INTR_CLEAR(dprintcreditmetrics, 1, DPR1_INT_CREDIT);
    CAPRI_INTR_CLEAR(dprintfifometrics, 0, DPR0_INT_FIFO);
    CAPRI_INTR_CLEAR(dprintfifometrics, 1, DPR1_INT_FIFO);
    CAPRI_INTR_CLEAR(dprintflopfifometrics, 0, DPR0_INT_FLOP_FIFO);
    CAPRI_INTR_CLEAR(dprintflopfifometrics, 1, DPR1_INT_FLOP_FIFO);
    CAPRI_INTR_CLEAR(dprintreg1metrics, 0, DPR0_INT_REG1);
    CAPRI_INTR_CLEAR(dprintreg2metrics, 0, DPR0_INT_REG2);
    CAPRI_INTR_CLEAR(dprintreg1metrics, 1, DPR1_INT_REG1);
    CAPRI_INTR_CLEAR(dprintreg2metrics, 1, DPR1_INT_REG2);
    CAPRI_INTR_CLEAR(intsparemetrics, 2, DPR0_INT_SPARE);
    CAPRI_INTR_CLEAR(intsparemetrics, 3, DPR1_INT_SPARE);
    CAPRI_INTR_CLEAR(dprintsramseccmetrics, 0, DPR0_INT_SRAMS_ECC);
    CAPRI_INTR_CLEAR(dprintsramseccmetrics, 1, DPR1_INT_SRAMS_ECC);
    CAPRI_INTR_CLEAR(ssepicsintbadaddrmetrics, 0, SSE_PICS_INT_BADADDR);
    CAPRI_INTR_CLEAR(ssepicsintbgmetrics, 0, SSE_PICS_INT_BG);
    CAPRI_INTR_CLEAR(ssepicsintpicsmetrics, 0, SSE_PICS_INT_PICS);
//    CAPRI_INTR_CLEAR(dbwaintdbmetrics, 0, DB_WA_INT_DB);
//    CAPRI_INTR_CLEAR(dbwaintlifqstatemapmetrics, 0, DB_WA_INT_LIF_QSTATE_MAP);
    CAPRI_INTR_CLEAR(sgeteinterrmetrics, 0, SGE_TE0_INT_ERR);
    CAPRI_INTR_CLEAR(sgeteinterrmetrics, 1, SGE_TE1_INT_ERR);
    CAPRI_INTR_CLEAR(sgeteinterrmetrics, 2, SGE_TE2_INT_ERR);
    CAPRI_INTR_CLEAR(sgeteinterrmetrics, 3, SGE_TE3_INT_ERR);
    CAPRI_INTR_CLEAR(sgeteinterrmetrics, 4, SGE_TE4_INT_ERR);
    CAPRI_INTR_CLEAR(sgeteinterrmetrics, 5, SGE_TE5_INT_ERR);
    CAPRI_INTR_CLEAR(sgeteintinfometrics, 0, SGE_TE0_INT_INFO);
    CAPRI_INTR_CLEAR(sgeteintinfometrics, 1, SGE_TE1_INT_INFO);
    CAPRI_INTR_CLEAR(sgeteintinfometrics, 2, SGE_TE2_INT_INFO);
    CAPRI_INTR_CLEAR(sgeteintinfometrics, 3, SGE_TE3_INT_INFO);
    CAPRI_INTR_CLEAR(sgeteintinfometrics, 4, SGE_TE4_INT_INFO);
    CAPRI_INTR_CLEAR(sgeteintinfometrics, 5, SGE_TE5_INT_INFO);
    CAPRI_INTR_CLEAR(sgempuinterrmetrics, 0, SGE_MPU0_INT_ERR);
    CAPRI_INTR_CLEAR(sgempuinterrmetrics, 1, SGE_MPU1_INT_ERR);
    CAPRI_INTR_CLEAR(sgempuinterrmetrics, 2, SGE_MPU2_INT_ERR);
    CAPRI_INTR_CLEAR(sgempuinterrmetrics, 3, SGE_MPU3_INT_ERR);
    CAPRI_INTR_CLEAR(sgempuinterrmetrics, 4, SGE_MPU4_INT_ERR);
    CAPRI_INTR_CLEAR(sgempuinterrmetrics, 5, SGE_MPU5_INT_ERR);
    CAPRI_INTR_CLEAR(sgempuintinfometrics, 0, SGE_MPU0_INT_INFO);
    CAPRI_INTR_CLEAR(sgempuintinfometrics, 1, SGE_MPU1_INT_INFO);
    CAPRI_INTR_CLEAR(sgempuintinfometrics, 2, SGE_MPU2_INT_INFO);
    CAPRI_INTR_CLEAR(sgempuintinfometrics, 3, SGE_MPU3_INT_INFO);
    CAPRI_INTR_CLEAR(sgempuintinfometrics, 4, SGE_MPU4_INT_INFO);
    CAPRI_INTR_CLEAR(sgempuintinfometrics, 5, SGE_MPU5_INT_INFO);
    CAPRI_INTR_CLEAR(mdhensintaxierrmetrics, 0, MD_HENS_INT_AXI_ERR);
    CAPRI_INTR_CLEAR(mdhensinteccmetrics, 0, MD_HENS_INT_DRBG_CRYPTORAM_ECC);
    CAPRI_INTR_CLEAR(mdhensinteccmetrics, 1, MD_HENS_INT_DRBG_INTRAM_ECC);
    CAPRI_INTR_CLEAR(mdhensintipcoremetrics, 0, MD_HENS_INT_IPCORE);
    CAPRI_INTR_CLEAR(mdhensinteccmetrics, 2, MD_HENS_INT_PK_ECC);
    CAPRI_INTR_CLEAR(mpmpnsintcryptometrics, 0, MP_MPNS_INT_CRYPTO);
    CAPRI_INTR_CLEAR(pbpbcintcreditunderflowmetrics, 0, PB_PBC_INT_CREDIT_UNDERFLOW);
    CAPRI_INTR_CLEAR(inteccdescmetrics, 0, PB_PBC_INT_ECC_DESC_0);
    CAPRI_INTR_CLEAR(inteccdescmetrics, 1, PB_PBC_INT_ECC_DESC_1);
    CAPRI_INTR_CLEAR(inteccdescmetrics, 2, PB_PBC_INT_ECC_FC_0);
    CAPRI_INTR_CLEAR(inteccdescmetrics, 3, PB_PBC_INT_ECC_FC_1);
    CAPRI_INTR_CLEAR(inteccdescmetrics, 4, PB_PBC_INT_ECC_LL_0);
    CAPRI_INTR_CLEAR(inteccdescmetrics, 5, PB_PBC_INT_ECC_LL_1);
    CAPRI_INTR_CLEAR(inteccdescmetrics, 6, PB_PBC_INT_ECC_NC);
    CAPRI_INTR_CLEAR(inteccdescmetrics, 7, PB_PBC_INT_ECC_PACK);
    CAPRI_INTR_CLEAR(inteccdescmetrics, 8, PB_PBC_INT_ECC_PORT_MON_IN);
    CAPRI_INTR_CLEAR(inteccdescmetrics, 9, PB_PBC_INT_ECC_PORT_MON_OUT);
    CAPRI_INTR_CLEAR(inteccdescmetrics, 10, PB_PBC_INT_ECC_RC);
    CAPRI_INTR_CLEAR(inteccdescmetrics, 11, PB_PBC_INT_ECC_RWR);
    CAPRI_INTR_CLEAR(inteccdescmetrics, 12, PB_PBC_INT_ECC_SCHED);
    CAPRI_INTR_CLEAR(inteccdescmetrics, 13, PB_PBC_INT_ECC_SIDEBAND);
    CAPRI_INTR_CLEAR(inteccdescmetrics, 14, PB_PBC_INT_ECC_UC);
    CAPRI_INTR_CLEAR(pbpbcintpbusviolationmetrics, 0, PB_PBC_INT_PBUS_VIOLATION_IN);
    CAPRI_INTR_CLEAR(pbpbcintpbusviolationmetrics, 1, PB_PBC_INT_PBUS_VIOLATION_OUT);
    CAPRI_INTR_CLEAR(pbpbcintrplmetrics, 0, PB_PBC_INT_ECC_RPL);
    CAPRI_INTR_CLEAR(pbpbcintwritemetrics, 0, PB_PBC_INT_ECC_WRITE_0);
    CAPRI_INTR_CLEAR(pbpbcintwritemetrics, 1, PB_PBC_INT_ECC_WRITE_1);
    CAPRI_INTR_CLEAR(pbpbcintwritemetrics, 2, PB_PBC_INT_ECC_WRITE_2);
    CAPRI_INTR_CLEAR(pbpbcintwritemetrics, 3, PB_PBC_INT_ECC_WRITE_3);
    CAPRI_INTR_CLEAR(pbpbcintwritemetrics, 4, PB_PBC_INT_ECC_WRITE_4);
    CAPRI_INTR_CLEAR(pbpbcintwritemetrics, 5, PB_PBC_INT_ECC_WRITE_5);
    CAPRI_INTR_CLEAR(pbpbcintwritemetrics, 6, PB_PBC_INT_ECC_WRITE_6);
    CAPRI_INTR_CLEAR(pbpbcintwritemetrics, 7, PB_PBC_INT_ECC_WRITE_7);
    CAPRI_INTR_CLEAR(pbpbcintwritemetrics, 8, PB_PBC_INT_ECC_WRITE_8);
    CAPRI_INTR_CLEAR(pbpbcintwritemetrics, 9, PB_PBC_INT_ECC_WRITE_9);
    CAPRI_INTR_CLEAR(pbpbcintwritemetrics, 10, PB_PBC_INT_ECC_WRITE_10);
    CAPRI_INTR_CLEAR(pbpbcintwritemetrics, 11, PB_PBC_INT_ECC_WRITE_11);
    CAPRI_INTR_CLEAR(inteccdescmetrics, 15, PB_PBC_HBM_INT_ECC_HBM_HT);
    CAPRI_INTR_CLEAR(inteccdescmetrics, 16, PB_PBC_HBM_INT_ECC_HBM_MTU);
    CAPRI_INTR_CLEAR(pbpbchbmintecchbmrbmetrics, 0, PB_PBC_HBM_INT_ECC_HBM_RB);
    CAPRI_INTR_CLEAR(inteccdescmetrics, 17, PB_PBC_HBM_INT_ECC_HBM_WB);
    CAPRI_INTR_CLEAR(pbpbchbminthbmaxierrrspmetrics, 0, PB_PBC_HBM_INT_ECC_HBM_AXI_ERR_RSP);
    CAPRI_INTR_CLEAR(pbpbchbminthbmdropmetrics, 0, PB_PBC_HBM_INT_ECC_HBM_DROP);
    CAPRI_INTR_CLEAR(pbpbchbminthbmpbusviolationmetrics, 0, PB_PBC_HBM_INT_ECC_HBM_PBUS_VIOLATION_IN);
    CAPRI_INTR_CLEAR(pbpbchbminthbmpbusviolationmetrics, 1, PB_PBC_HBM_INT_ECC_HBM_PBUS_VIOLATION_OUT);
    CAPRI_INTR_CLEAR(pbpbchbminthbmxoffmetrics, 0, PB_PBC_HBM_INT_ECC_HBM_XOFF);
//    CAPRI_INTR_CLEAR(inteccdescmetrics, 18, PM_PBM_INT_ECC_COL0);
//    CAPRI_INTR_CLEAR(inteccdescmetrics, 19, PM_PBM_INT_ECC_COL1);
//    CAPRI_INTR_CLEAR(inteccdescmetrics, 20, PM_PBM_INT_ECC_COL2);
//    CAPRI_INTR_CLEAR(inteccdescmetrics, 21, PM_PBM_INT_ECC_COL3);
//    CAPRI_INTR_CLEAR(inteccdescmetrics, 22, PM_PBM_INT_ECC_COL4);
//    CAPRI_INTR_CLEAR(inteccdescmetrics, 23, PM_PBM_INT_ECC_COL5);
    CAPRI_INTR_CLEAR(mcmchintmcmetrics, 0, MC0_MCH_INT_MC_INTREG);
    CAPRI_INTR_CLEAR(mcmchintmcmetrics, 1, MC1_MCH_INT_MC_INTREG);
    CAPRI_INTR_CLEAR(mcmchintmcmetrics, 2, MC2_MCH_INT_MC_INTREG);
    CAPRI_INTR_CLEAR(mcmchintmcmetrics, 3, MC3_MCH_INT_MC_INTREG);
    CAPRI_INTR_CLEAR(mcmchintmcmetrics, 4, MC4_MCH_INT_MC_INTREG);
    CAPRI_INTR_CLEAR(mcmchintmcmetrics, 5, MC5_MCH_INT_MC_INTREG);
    CAPRI_INTR_CLEAR(mcmchintmcmetrics, 6, MC6_MCH_INT_MC_INTREG);
    CAPRI_INTR_CLEAR(mcmchintmcmetrics, 7, MC7_MCH_INT_MC_INTREG);
}
