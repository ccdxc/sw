/***********************************************************************/
/* rdma_req_txdma.p4 */
/***********************************************************************/

#include "../common-p4+/common_txdma_dummy.p4"

/**** table declarations ****/

#define tx_table_s0_t0 req_tx_s0_t0
#define tx_table_s0_t1 req_tx_s0_t1
#define tx_table_s0_t2 req_tx_s0_t2
#define tx_table_s0_t3 req_tx_s0_t3

#define tx_table_s1_t0 req_tx_s1_t0
#define tx_table_s1_t1 req_tx_s1_t1
#define tx_table_s1_t2 req_tx_s1_t2
#define tx_table_s1_t3 req_tx_s1_t3

#define tx_table_s2_t0 req_tx_s2_t0
#define tx_table_s2_t1 req_tx_s2_t1
#define tx_table_s2_t2 req_tx_s2_t2
#define tx_table_s2_t3 req_tx_s2_t3

#define tx_table_s3_t0 req_tx_s3_t0
#define tx_table_s3_t1 req_tx_s3_t1
#define tx_table_s3_t2 req_tx_s3_t2
#define tx_table_s3_t3 req_tx_s3_t3

#define tx_table_s4_t0 req_tx_s4_t0
#define tx_table_s4_t1 req_tx_s4_t1
#define tx_table_s4_t2 req_tx_s4_t2
#define tx_table_s4_t3 req_tx_s4_t3

#define tx_table_s5_t0 req_tx_s5_t0
#define tx_table_s5_t1 req_tx_s5_t1
#define tx_table_s5_t2 req_tx_s5_t2
#define tx_table_s5_t3 req_tx_s5_t3

#define tx_table_s6_t0 req_tx_s6_t0
#define tx_table_s6_t1 req_tx_s6_t1
#define tx_table_s6_t2 req_tx_s6_t2
#define tx_table_s6_t3 req_tx_s6_t3

#define tx_table_s7_t0 req_tx_s7_t0
#define tx_table_s7_t1 req_tx_s7_t1
#define tx_table_s7_t2 req_tx_s7_t2
#define tx_table_s7_t3 req_tx_s7_t3


#define tx_table_s0_t0_action req_tx_sqcb_process
#define tx_table_s0_t0_action1 req_tx_stage0_recirc_action

#define tx_table_s1_t0_action req_tx_dummy_sqpt_process
#define tx_table_s1_t0_action1 req_tx_sqpt_process
#define tx_table_s1_t0_action3 req_tx_bktrack_sqcb2_process
#define tx_table_s1_t0_action4 req_tx_sqcb2_cnp_process
#define tx_table_s1_t0_action5 req_tx_sqcb2_fence_process
#define tx_table_s1_t0_action  req_tx_sqlkey_recirc_process_t0
#define tx_table_s1_t2_action req_tx_sqsge_iterate_process_s1

#define tx_table_s2_t0_action  req_tx_sqwqe_process
#define tx_table_s2_t0_action1 req_tx_bktrack_sqwqe_process_s2
#define tx_table_s2_t0_action4 req_tx_bktrack_sqpt_process
#define tx_table_s2_t0_action6 req_tx_dcqcn_cnp_process
#define tx_table_s2_t1_action  req_tx_bktrack_sqcb1_write_back_process
#define tx_table_s2_t2_action req_tx_sqsge_iterate_process_s2

#define tx_table_s3_t0_action  req_tx_sqsge_process 
#define tx_table_s3_t0_action1 req_tx_bktrack_sqwqe_process_s3
#define tx_table_s3_t0_action2 req_tx_bktrack_sqsge_process_s3
#define tx_table_s3_t0_action4 req_tx_sqlkey_invalidate_process_s3
#define tx_table_s3_t2_action  req_tx_dcqcn_enforce_process_s3
#define tx_table_s3_t3_action1 req_tx_load_ah_size_process

#define tx_table_s4_t0_action  req_tx_sqlkey_process_t0
#define tx_table_s4_t0_action1 req_tx_bktrack_sqwqe_process_s4
#define tx_table_s4_t0_action2 req_tx_bktrack_sqsge_process_s4
#define tx_table_s4_t0_action4 req_tx_sqlkey_invalidate_process_t0
#define tx_table_s4_t0_action5 req_tx_bind_mw_sqlkey_process_s4
#define tx_table_s4_t0_action6 req_tx_bind_mw_rkey_process_s4
#define tx_table_s4_t0_action7 req_tx_frpmr_sqlkey_process_t0
#define tx_table_s4_t1_action  req_tx_sqlkey_process_t1
#define tx_table_s4_t1_action1 req_tx_sqlkey_invalidate_process_t1
#define tx_table_s4_t1_action2 req_tx_frpmr_sqlkey_process_t1
#define tx_table_s4_t2_action  req_tx_dcqcn_enforce_process_s4

#define tx_table_s5_t0_action  req_tx_sqptseg_process_t0
#define tx_table_s5_t0_action1 req_tx_bktrack_sqwqe_process_s5
#define tx_table_s5_t0_action2 req_tx_bktrack_sqsge_process_s5
#define tx_table_s5_t0_action4 req_tx_load_hdr_template_process
#define tx_table_s5_t1_action  req_tx_sqptseg_process_t1
#define tx_table_s5_t2_action  req_tx_add_headers_process_rd
#define tx_table_s5_t2_action1 req_tx_add_headers_process_send_wr
#define tx_table_s5_t2_action2 req_tx_sqcb2_write_back_process
#define tx_table_s5_t2_action3 req_tx_timer_expiry_process
#define tx_table_s5_t2_action4 req_tx_sq_drain_feedback_process
#define tx_table_s5_t2_action5 req_tx_credits_process

#define tx_table_s6_t0_action1 req_tx_bktrack_sqwqe_process_s6
#define tx_table_s6_t0_action2 req_tx_bktrack_sqsge_process_s6
#define tx_table_s6_t2_action  req_tx_write_back_process
#define tx_table_s6_t2_action1 req_tx_write_back_process_rd
#define tx_table_s6_t2_action2 req_tx_write_back_process_send_wr
#define tx_table_s6_t2_action3 req_tx_frpmr_write_back_process
#define tx_table_s6_t3_action  req_tx_add_headers_2_process

#define tx_table_s7_t0_action3 req_tx_bktrack_write_back_process
#define tx_table_s7_t1_action4 req_tx_bktrack_sqcb2_write_back_process

#define tx_table_s7_t3_action  req_tx_stats_process

#include "../common-p4+/common_txdma.p4"
#include "./rdma_txdma_headers.p4"

/**** Macros ****/

#define GENERATE_GLOBAL_K \
    modify_field(phv_global_common_scr.lif, phv_global_common.lif);\
    modify_field(phv_global_common_scr.qid, phv_global_common.qid);\
    modify_field(phv_global_common_scr.qtype, phv_global_common.qtype);\
    modify_field(phv_global_common_scr.cb_addr, phv_global_common.cb_addr);\
    modify_field(phv_global_common_scr.pt_base_addr_page_id, phv_global_common.pt_base_addr_page_id);\
    modify_field(phv_global_common_scr.log_num_pt_entries, phv_global_common.log_num_pt_entries);\
    modify_field(phv_global_common_scr.spec_cindex, phv_global_common.spec_cindex);\
    modify_field(phv_global_common_scr.rsvd, phv_global_common.rsvd);\
    modify_field(phv_global_common_scr._ud, phv_global_common._ud);\
    modify_field(phv_global_common_scr._inline, phv_global_common._inline);\
    modify_field(phv_global_common_scr._rsvd2, phv_global_common._rsvd2);\
    modify_field(phv_global_common_scr._rexmit, phv_global_common._rexmit);\
    modify_field(phv_global_common_scr._inv_rkey, phv_global_common._inv_rkey);\
    modify_field(phv_global_common_scr._immdt, phv_global_common._immdt);\
    modify_field(phv_global_common_scr._atomic_cswap, phv_global_common._atomic_cswap);\
    modify_field(phv_global_common_scr._atomic_fna, phv_global_common._atomic_fna);\
    modify_field(phv_global_common_scr._write, phv_global_common._write);\
    modify_field(phv_global_common_scr._read_req, phv_global_common._read_req);\
    modify_field(phv_global_common_scr._send, phv_global_common._send);\
    modify_field(phv_global_common_scr._only, phv_global_common._only);\
    modify_field(phv_global_common_scr._last, phv_global_common._last);\
    modify_field(phv_global_common_scr._middle, phv_global_common._middle);\
    modify_field(phv_global_common_scr._first, phv_global_common._first);\
    modify_field(phv_global_common_scr._error_disable_qp, phv_global_common._error_disable_qp);\


/**** header definitions ****/

header_type phv_global_common_t {
    fields {
        lif                              :   11;
        qid                              :   24;
        qtype                            :    3;
        cb_addr                          :   25;
        pt_base_addr_page_id             :   22;
        log_num_pt_entries               :    5;
        spec_cindex                      :   16;
        rsvd                             :    6;
        _ud                              :    1;
        _inline                          :    1;
        _rsvd2                           :    1;
        _rexmit                          :    1;
        _inv_rkey                        :    1;
        _immdt                           :    1;
        _atomic_cswap                    :    1;
        _atomic_fna                      :    1;
        _write                           :    1;
        _read_req                        :    1;
        _send                            :    1;
        _only                            :    1;
        _last                            :    1;
        _middle                          :    1;
        _first                           :    1;
        _error_disable_qp                :    1;
    }
}

header_type req_tx_bktrack_sqcb2_write_back_info_t {
    fields {
        tx_psn                           :   24;
        ssn                              :   24;
        skip_wqe_start_psn               :    1;
        imm_data_or_inv_key              :   32;
        pad1                             :   32;
        op_type                          :    4;
        sq_cindex                        :   16;
        in_progress                      :    1;
        bktrack_in_progress              :    1;
        msg_psn                          :   24;
        pad                              :   1;
    }
}

header_type roce_options_t {
    fields {
        OCS_kind                         :    8;
        OCS_value                        :    8;
        TS_kind                          :    8;
        TS_len                           :    8;
        TS_value                         :   32;
        TS_echo                          :   32;
        MSS_kind                         :    8;
        MSS_len                          :    8;
        MSS_value                        :   16;
        EOL_kind                         :    8;
    }
}

header_type req_tx_rrqwqe_to_hdr_info_t {
    fields {
        rsvd                             :    2;
        last_pkt                         :    1;
        op_type                          :    4;
        first                            :    1;
        tbl_id                           :    3;
        log_pmtu                         :    5;
        rrq_p_index                      :    8;
    }
}

header_type req_tx_to_stage_bt_info_t {
    fields {
        wqe_addr                         :   64;
        log_pmtu                         :    5;
        log_sq_page_size                 :    5;
        log_wqe_size                     :    5;
        log_num_wqes                     :    5;
        rexmit_psn                       :   24;
        pad                              :   11;
        // These fields are written in tx_stage0_load_params. Do not move.
        log_num_kt_entries               :    5;
        log_num_dcqcn_profiles           :    4;
    }
}

header_type req_tx_to_stage_bt_wb_info_t {
    fields {
        wqe_addr                         :   64;
        wqe_start_psn                    :   24;
    }
}

header_type req_tx_to_stage_stats_info_t {
    fields {
        pyld_bytes                       :   16;
        npg                              :    1;
        npg_bindmw_t1                    :    1;
        npg_bindmw_t2                    :    1;
        npg_frpmr                        :    1;
        npg_local_inv                    :    1;
        timeout                          :    1;
        timeout_local_ack                :    1;
        timeout_rnr                      :    1;
        sq_drain                         :    1;
        lif_cqe_error_id_vld             :    1;
        lif_error_id_vld                 :    1;
        lif_error_id                     :    4;
        qp_err_disabled                  :    1;
        qp_err_dis_flush_rq              :    1;
        qp_err_dis_ud_pmtu               :    1;
        qp_err_dis_ud_fast_reg           :    1;
        qp_err_dis_ud_priv               :    1;
        qp_err_dis_no_dma_cmds           :    1;
        qp_err_dis_lkey_inv_state        :    1;
        qp_err_dis_lkey_inv_pd           :    1;
        qp_err_dis_lkey_rsvd_lkey        :    1;
        qp_err_dis_lkey_access_violation :    1;
        qp_err_dis_bind_mw_len_exceeded          : 1;
        qp_err_dis_bind_mw_rkey_inv_pd           : 1;
        qp_err_dis_bind_mw_rkey_inv_zbva         : 1;
        qp_err_dis_bind_mw_rkey_inv_len          : 1;
        qp_err_dis_bind_mw_rkey_inv_mw_state     : 1;
        qp_err_dis_bind_mw_rkey_type_disallowed  : 1;
        qp_err_dis_bind_mw_lkey_state_valid      : 1;
        qp_err_dis_bind_mw_lkey_no_bind          : 1;
        qp_err_dis_bind_mw_lkey_zero_based       : 1;
        qp_err_dis_bind_mw_lkey_invalid_acc_ctrl : 1;
        qp_err_dis_bind_mw_lkey_invalid_va       : 1;
        qp_err_dis_bktrack_inv_num_sges          : 1;
        qp_err_dis_bktrack_inv_rexmit_psn        : 1;
        qp_err_dis_frpmr_fast_reg_not_enabled    : 1;
        qp_err_dis_frpmr_invalid_pd              : 1;
        qp_err_dis_frpmr_invalid_state           : 1;
        qp_err_dis_frpmr_invalid_len             : 1;
        qp_err_dis_frpmr_ukey_not_enabled        : 1;
        qp_err_dis_inv_lkey_qp_mismatch          : 1;
        qp_err_dis_inv_lkey_pd_mismatch          : 1;
        qp_err_dis_inv_lkey_invalid_state        : 1;
        qp_err_dis_inv_lkey_inv_not_allowed      : 1;
        qp_err_dis_table_error                   : 1;
        qp_err_dis_phv_intrinsic_error           : 1;
        qp_err_dis_table_resp_error              : 1;
        qp_err_dis_inv_optype                    : 1;
        qp_err_dis_err_retry_exceed              : 1;
        qp_err_dis_rnr_retry_exceed              : 1;
        qp_err_dis_rsvd                          : 26;
        dcqcn_cnp_sent                           : 1;
        np_cnp_sent                              : 1;
        rp_num_byte_threshold_db                 : 1;
        pad                                      : 30;
    }
}


header_type req_tx_wqe_to_sge_info_t {
    fields {
        in_progress                      : 1;
        op_type                          : 4;
        first                            : 1;
        current_sge_id                   : 8;
        num_valid_sges                   : 8;
        current_sge_offset               : 32;
        remaining_payload_bytes          : 16;
        poll_in_progress                 : 1;
        color                            : 1;
        spec_enable                      : 1;
        rsvd                             : 17;
        dma_cmd_start_index              : 6;
        imm_data_or_inv_key              : 32;
        ah_handle                        : 32;
    }
}

header_type req_tx_sq_bktrack_info_t {
    fields {
        sq_c_index                          : 16;
        in_progress                         : 1;
        current_sge_id                      : 8;
        num_sges                            : 8;
        current_sge_offset                  : 32;
        tx_psn                              : 24;
        ssn                                 : 24;
        op_type                             : 4;
        sq_p_index_or_imm_data1_or_inv_key1 : 16;
        imm_data2_or_inv_key2               : 16;
        spec_enable                         : 1;
        pad                                 : 10;
    }
}

header_type req_tx_bktrack_sqcb1_write_back_info_t {
    fields {
        tx_psn                           : 24;
        ssn                              : 24;
    }
}

header_type req_tx_sqcb0_to_sqcb2_info_t {
    fields {
        sq_c_index                       :   16;
        sq_p_index                       :   16;
        in_progress                      :    1;
        bktrack_in_progress              :    1;
        current_sge_id                   :    8;
        num_sges                         :    8;
        current_sge_offset               :   32;
        update_credits                   :    1;
        bktrack                          :    1;
        pt_base_addr                     :   40;
        op_type                          :    4;
        sq_in_hbm                        :    1;
        skip_pt                          :    1;
        spec_enable                      :    1;
        pad                              :   29;
    }
}

header_type req_tx_hdr_template_t {
    fields {
        data                             :  512;
    }
}

header_type req_tx_sqcb_write_back_info_t {
    fields {
        hdr_template_inline              :    1;
        busy                             :    1;
        in_progress                      :    1;
        bktrack_in_progress              :    1;
        op_type                          :    4;
        first                            :    1;
        last_pkt                         :    1;
        set_fence                        :    1;
        set_li_fence                     :    1;
        empty_rrq_bktrack                :    1;
        current_sge_offset               :   32;
        current_sge_id                   :    8;
        num_sges                         :    8;
        sq_c_index                       :   16;
        ah_size                          :    8;
        poll_in_progress                 :    1;
        color                            :    1;
        poll_failed                      :    1;
        rate_enforce_failed              :    1;
        non_packet_wqe                   :    1;
        credit_check_failed              :    1;
        drop_phv                         :    1;
        drop_bktrack                     :    1;
        rsvd                             :    3;
        op_info                          :   64;
    }
}

header_type req_tx_sqcb_write_back_info_rd_t {
    fields {
        hdr_template_inline              :    1;
        busy                             :    1;
        in_progress                      :    1;
        bktrack_in_progress              :    1;
        op_type                          :    4;
        first                            :    1;
        last_pkt                         :    1;
        set_fence                        :    1;
        set_li_fence                     :    1;
        empty_rrq_bktrack                :    1;
        current_sge_offset               :   32;
        current_sge_id                   :    8;
        num_sges                         :    8;
        sq_c_index                       :   16;
        ah_size                          :    8;
        poll_in_progress                 :    1;
        color                            :    1;
        poll_failed                      :    1;
        rate_enforce_failed              :    1;
        non_packet_wqe                   :    1;
        credit_check_failed              :    1;
        drop_phv                         :    1;
        rsvd                             :    4;
        op_rd_read_len                   :   32;
        op_rd_log_pmtu                   :    5;
        op_rd_pad                        :   27;
    }
}

header_type req_tx_sqcb_write_back_info_send_wr_t {
    fields {
        hdr_template_inline              :    1;
        busy                             :    1;
        in_progress                      :    1;
        bktrack_in_progress              :    1;
        op_type                          :    4;
        first                            :    1;
        last_pkt                         :    1;
        set_fence                        :    1;
        set_li_fence                     :    1;
        empty_rrq_bktrack                :    1;
        current_sge_offset               :   32;
        current_sge_id                   :    8;
        num_sges                         :    8;
        sq_c_index                       :   16;
        ah_size                          :    8;
        poll_in_progress                 :    1;
        color                            :    1;
        poll_failed                      :    1;
        rate_enforce_failed              :    1;
        non_packet_wqe                   :    1;
        credit_check_failed              :    1;
        drop_phv                         :    1;
        rsvd                             :    4;
        op_send_wr_imm_data_or_inv_key   :   32;
        op_send_wr_ah_handle             :   32;
    }
}

header_type req_tx_s0_t {
    fields {
        lif                              :   11;
        qid                              :   24;
        qtype                            :    3;
    }
}

header_type req_tx_sqcb_to_wqe_info_t {
    fields {
        in_progress                      :    1;
        spec_enable                      :    1;
        log_pmtu                         :    5;
        li_fence_cleared                 :    1;
        current_sge_id                   :    8;
        num_valid_sges                   :    8;
        current_sge_offset               :   32;
        remaining_payload_bytes          :   16;
        rrq_p_index                      :    8;
        pd                               :   32;
        poll_in_progress                 :    1;
        color                            :    1;
        fence_done                       :    1;
        frpmr_lkey_state_upd             :    1;
        dma_cmd_start_index              :    6;
        pad                              :   38;
    }
}

header_type req_tx_to_stage_fence_info_t {
    fields {
        wqe_addr                         : 64;
        bktrack_fence_marker_phv         : 1;
        pad                              : 63;
    }
}

header_type req_tx_to_stage_sqwqe_info_t {
    fields {
        wqe_addr                         :   64;
        header_template_addr             :   32;
        fast_reg_rsvd_lkey_enable        :    1;
        pad                              :   22;
        // These fields are written in tx_stage0_load_params. Do not move.
        log_num_kt_entries               :    5;
        log_num_dcqcn_profiles           :    4;
    }
}

// req_tx_dcqcn_enforce_process.s runs in both s3 as well
// as s4. So the offset of to_s3 fields and to_s4 fields
// should be same. Any additional field used in s4 should
// be at the bottom
header_type req_tx_to_stage_sqsge_info_t {
    fields {
        header_template_addr             : 32;
        packet_len                       : 14;
        congestion_mgmt_type             : 2;
        priv_oper_enable                 : 1;
        pad                              : 15;
        spec_msg_psn                     : 24;
        spec_enable                      : 1;
    }
}

header_type req_tx_to_stage_dcqcn_bind_mw_info_t {
    fields {
        header_template_addr_or_pd       : 32; 
        packet_len                       : 14;
        congestion_mgmt_type             :  2; 
        host_addr_spec_enable            :  1;
        pad                              : 14;
        spec_reset                       :  1;
        mr_l_key                         : 32; 
        mr_cookie_msg_psn                : 32; 
    }
}

header_type req_tx_to_stage_frpmr_sqlkey_info_t {
    fields {
        header_template_addr_or_pd       : 32;
        packet_len                       : 14;
        congestion_mgmt_type             :  2;
        rsvd                             : 16;
        len                              : 64;
    }
}


header_type req_tx_to_stage_sqcb_wb_add_hdr_info_t {
    fields {
        wqe_addr                         : 64;
        pad                              : 16;
        ah_size                          : 8;
        spec_enable                      : 1;
        fence                            : 1;
        packet_len                       : 14;
        read_req_adjust                  : 24;
    }
}

// Note- wqe_addr should be at same offset in req_tx_to_stage_frpmr_sqcb_wb_info_t 
// and req_tx_to_stage_sqcb_wb_info_t since both are loaded in stage5.
header_type req_tx_to_stage_frpmr_sqcb_wb_info_t {
    fields {
        wqe_addr                         : 64;
        frpmr_dma_src_addr               : 64;
    }
}

//header_type req_tx_sq_to_stage_t {
//    fields {
//        wqe_addr                         :   64;
//        spec_cindex                      :   16;
//        header_template_addr             :   32;
//       packet_len                       :   14;
//        congestion_mgmt_type           :    1;
//        fence                            :    1;
//    }
//}

header_type rd_t {
    fields {
        read_len                         :   32;
        log_pmtu                         :    5;
        pad                              :   27;
    }
}

header_type req_tx_sqcb_to_pt_info_t {
    fields {
        page_offset                      :   16;
        page_seg_offset                  :    3;
        remaining_payload_bytes          :   16;
        rrq_p_index                      :    8;
        pd                               :   32;
        log_pmtu                         :    5;
        poll_in_progress                 :    1;
        color                            :    1;
        read_req_adjust                  :   32;
        spec_enable                      :    1;
        pad                              :   45;
    }
}

header_type req_tx_add_hdr_info_t {
    fields {
        hdr_template_inline              :    1;
        service                          :    4;
        header_template_addr             :   32;
        header_template_size             :    8;
        roce_opt_ts_enable               :    1;
        roce_opt_mss_enable              :    1;
        pad                              :  113;
    }
}

header_type req_tx_lkey_to_ptseg_info_t {
    fields {
        pt_offset                        :   32;
        log_page_size                    :    5;
        pt_bytes                         :   16;
        dma_cmd_start_index              :    8;
        sge_index                        :    8;
        host_addr                        :    1;
        pad                              :   90;
    }
}

header_type req_tx_sge_to_lkey_info_t {
    fields {
        sge_va                           :   64;
        sge_bytes                        :   16;
        dma_cmd_start_index              :    8;
        sge_index                        :    8;
        lkey_invalidate                  :    1;
        rsvd_key_err                     :    1;
        pad                              :   62;
    }
}

header_type req_tx_sqwqe_to_lkey_inv_info_t {
    fields {
        sge_index                        :    8;
        set_li_fence                     :    1;
        pad                              :  151;
    }
}

header_type req_tx_sqwqe_to_lkey_frpmr_info_t {
    fields {
        sge_index                        :    1;
        fast_reg_rsvd_lkey_enable        :    1;
        zbva                             :    1;
        lkey_state_update                :    1;
        rsvd1                            :    3;
        rsvd3                            :    1;
        new_user_key                     :    8;
        log_page_size                    :    8;
        num_pt_entries                   :    32;
        acc_ctrl                         :    8;
        base_va                          :    64;
        rsvd2                            :    32;
    }
}

header_type req_tx_frpmr_lkey_to_wb_info_t {
    fields {
        pt_base                          :    32;
        dma_size                         :    32;
        rsvd2                            :    96;
    }
}

header_type req_tx_sqwqe_to_lkey_mw_info_t {
    fields {
        va                               : 64;
        len                              : 32; // TODO 64b
        r_key                            : 32;
        new_r_key_key                    : 8;
        acc_ctrl                         : 8;
        mw_type                          : 2;
        zbva                             : 1;
        pad                              : 13;
    }
}

header_type req_tx_sqlkey_to_rkey_mw_info_t {
    fields {
        va                               : 64;
        len                              : 32; // TODO 64b
        mw_pt_base                       : 32;
        new_r_key_key                    : 8;
        acc_ctrl                         : 8;
        mw_type                          : 2;
        zbva                             : 1;
        log_page_size                    : 8;
        pad                              : 5;
    }
}

header_type req_tx_stats_info_t {
    fields {
        pad                              :  160;
    }
}

/**** header unions and scratch ****/

@pragma pa_header_union ingress app_header rdma_recirc

/* metadata common_global_phv_t common_phv; */
metadata roce_recirc_header_t rdma_recirc;

@pragma scratch_metadata
/* metadata common_global_phv_t common_global_scratch; */
metadata roce_recirc_header_t rdma_recirc_scr;


/**** global header unions ****/

@pragma pa_header_union ingress common_global
metadata phv_global_common_t phv_global_common;
@pragma scratch_metadata
metadata phv_global_common_t phv_global_common_scr;

/**** to stage header unions ****/

//To-Stage-0
@pragma pa_header_union ingress to_stage_0 to_s0_sqsge_info

metadata req_tx_to_stage_sqsge_info_t to_s0_sqsge_info;
@pragma scratch_metadata
metadata req_tx_to_stage_sqsge_info_t to_s0_sqsge_info_scr;


//To-Stage-1
@pragma pa_header_union ingress to_stage_1 to_s1_fence_info to_s1_bt_info to_s1_dcqcn_bind_mw_info

metadata req_tx_to_stage_bt_info_t to_s1_bt_info;
@pragma scratch_metadata
metadata req_tx_to_stage_bt_info_t to_s1_bt_info_scr;

metadata req_tx_to_stage_fence_info_t to_s1_fence_info;
@pragma scratch_metadata
metadata req_tx_to_stage_fence_info_t to_s1_fence_info_scr;

metadata req_tx_to_stage_dcqcn_bind_mw_info_t to_s1_dcqcn_bind_mw_info;
@pragma scratch_metadata
metadata req_tx_to_stage_dcqcn_bind_mw_info_t to_s1_dcqcn_bind_mw_info_scr;

//To-Stage-2
@pragma pa_header_union ingress to_stage_2 to_s2_sqwqe_info to_s2_bt_info

metadata req_tx_to_stage_sqwqe_info_t to_s2_sqwqe_info;
@pragma scratch_metadata
metadata req_tx_to_stage_sqwqe_info_t to_s2_sqwqe_info_scr;

metadata req_tx_to_stage_bt_info_t to_s2_bt_info;
@pragma scratch_metadata
metadata req_tx_to_stage_bt_info_t to_s2_bt_info_scr;

//To-Stage-3
@pragma pa_header_union ingress to_stage_3 to_s3_sqsge_info to_s3_bt_info

metadata req_tx_to_stage_bt_info_t to_s3_bt_info;
@pragma scratch_metadata
metadata req_tx_to_stage_bt_info_t to_s3_bt_info_scr;

metadata req_tx_to_stage_sqsge_info_t to_s3_sqsge_info;
@pragma scratch_metadata
metadata req_tx_to_stage_sqsge_info_t to_s3_sqsge_info_scr;

//To-Stage-4
@pragma pa_header_union ingress to_stage_4 to_s4_dcqcn_bind_mw_info to_s4_bt_info to_s4_frpmr_sqlkey_info

metadata req_tx_to_stage_bt_info_t to_s4_bt_info;
@pragma scratch_metadata
metadata req_tx_to_stage_bt_info_t to_s4_bt_info_scr;

metadata req_tx_to_stage_dcqcn_bind_mw_info_t to_s4_dcqcn_bind_mw_info;
@pragma scratch_metadata
metadata req_tx_to_stage_dcqcn_bind_mw_info_t to_s4_dcqcn_bind_mw_info_scr;

metadata req_tx_to_stage_frpmr_sqlkey_info_t to_s4_frpmr_sqlkey_info;
@pragma scratch_metadata
metadata req_tx_to_stage_frpmr_sqlkey_info_t to_s4_frpmr_sqlkey_info_scr;

//To-Stage-5
@pragma pa_header_union ingress to_stage_5 to_s5_sqcb_wb_add_hdr_info to_s5_bt_info

metadata req_tx_to_stage_bt_info_t to_s5_bt_info;
@pragma scratch_metadata
metadata req_tx_to_stage_bt_info_t to_s5_bt_info_scr;

metadata req_tx_to_stage_sqcb_wb_add_hdr_info_t to_s5_sqcb_wb_add_hdr_info;
@pragma scratch_metadata
metadata req_tx_to_stage_sqcb_wb_add_hdr_info_t to_s5_sqcb_wb_add_hdr_info_scr;

//To-Stage-6
@pragma pa_header_union ingress to_stage_6 to_s6_sqcb_wb_add_hdr_info to_s6_bt_info to_s6_frpmr_sqcb_wb_info

metadata req_tx_to_stage_bt_info_t to_s6_bt_info;
@pragma scratch_metadata
metadata req_tx_to_stage_bt_info_t to_s6_bt_info_scr;

metadata req_tx_to_stage_sqcb_wb_add_hdr_info_t to_s6_sqcb_wb_add_hdr_info;
@pragma scratch_metadata
metadata req_tx_to_stage_sqcb_wb_add_hdr_info_t to_s6_sqcb_wb_add_hdr_info_scr;

metadata req_tx_to_stage_frpmr_sqcb_wb_info_t to_s6_frpmr_sqcb_wb_info;
@pragma scratch_metadata
metadata req_tx_to_stage_frpmr_sqcb_wb_info_t to_s6_frpmr_sqcb_wb_info_scr;

//To-Stage-7
@pragma pa_header_union ingress to_stage_7 to_s7_bt_wb_info to_s7_stats_info

metadata req_tx_to_stage_bt_wb_info_t to_s7_bt_wb_info;
@pragma scratch_metadata
metadata req_tx_to_stage_bt_wb_info_t to_s7_bt_wb_info_scr;

metadata req_tx_to_stage_stats_info_t to_s7_stats_info;
@pragma scratch_metadata
metadata req_tx_to_stage_stats_info_t to_s7_stats_info_scr;

/**** stage to stage header unions ****/

//Table-0
@pragma pa_header_union ingress common_t0_s2s t0_s2s_sqcb_to_wqe_info t0_s2s_sqcb_to_pt_info t0_s2s_sqcb0_to_sqcb2_info t0_s2s_wqe_to_sge_info t0_s2s_sq_bktrack_info t0_s2s_sqcb_write_back_info t0_s2s_sge_to_lkey_info t0_s2s_lkey_to_ptseg_info t0_s2s_sqwqe_to_lkey_inv_info t0_s2s_sqwqe_to_lkey_mw_info t0_s2s_sqlkey_to_rkey_mw_info t0_s2s_sqwqe_to_lkey_frpmr_info

metadata req_tx_sqcb_to_wqe_info_t t0_s2s_sqcb_to_wqe_info;
@pragma scratch_metadata
metadata req_tx_sqcb_to_wqe_info_t t0_s2s_sqcb_to_wqe_info_scr;

metadata req_tx_sqcb_to_pt_info_t t0_s2s_sqcb_to_pt_info;
@pragma scratch_metadata
metadata req_tx_sqcb_to_pt_info_t t0_s2s_sqcb_to_pt_info_scr;

metadata req_tx_sqcb0_to_sqcb2_info_t t0_s2s_sqcb0_to_sqcb2_info;
@pragma scratch_metadata
metadata req_tx_sqcb0_to_sqcb2_info_t t0_s2s_sqcb0_to_sqcb2_info_scr;

metadata req_tx_wqe_to_sge_info_t t0_s2s_wqe_to_sge_info;
@pragma scratch_metadata
metadata req_tx_wqe_to_sge_info_t t0_s2s_wqe_to_sge_info_scr;

metadata req_tx_sq_bktrack_info_t t0_s2s_sq_bktrack_info;
@pragma scratch_metadata
metadata req_tx_sq_bktrack_info_t t0_s2s_sq_bktrack_info_scr;

metadata req_tx_sqcb_write_back_info_t t0_s2s_sqcb_write_back_info;
@pragma scratch_metadata
metadata req_tx_sqcb_write_back_info_t t0_s2s_sqcb_write_back_info_scr;

metadata req_tx_sge_to_lkey_info_t t0_s2s_sge_to_lkey_info;
@pragma scratch_metadata
metadata req_tx_sge_to_lkey_info_t t0_s2s_sge_to_lkey_info_scr;

metadata req_tx_lkey_to_ptseg_info_t t0_s2s_lkey_to_ptseg_info;
@pragma scratch_metadata
metadata req_tx_lkey_to_ptseg_info_t t0_s2s_lkey_to_ptseg_info_scr;

metadata req_tx_sqwqe_to_lkey_inv_info_t t0_s2s_sqwqe_to_lkey_inv_info;
@pragma scratch_metadata
metadata req_tx_sqwqe_to_lkey_inv_info_t t0_s2s_sqwqe_to_lkey_inv_info_scr;

metadata req_tx_sqwqe_to_lkey_mw_info_t t0_s2s_sqwqe_to_lkey_mw_info;
@pragma scratch_metadata
metadata req_tx_sqwqe_to_lkey_mw_info_t t0_s2s_sqwqe_to_lkey_mw_info_scr;

metadata req_tx_sqlkey_to_rkey_mw_info_t t0_s2s_sqlkey_to_rkey_mw_info;
@pragma scratch_metadata
metadata req_tx_sqlkey_to_rkey_mw_info_t t0_s2s_sqlkey_to_rkey_mw_info_scr;

metadata req_tx_sqwqe_to_lkey_frpmr_info_t t0_s2s_sqwqe_to_lkey_frpmr_info;
@pragma scratch_metadata
metadata req_tx_sqwqe_to_lkey_frpmr_info_t t0_s2s_sqwqe_to_lkey_frpmr_info_scr;

//Table-1
@pragma pa_header_union ingress common_t1_s2s t1_s2s_bktrack_sqcb2_write_back_info t1_s2s_sge_to_lkey_info t1_s2s_lkey_to_ptseg_info t1_s2s_bktrack_sqcb1_write_back_info t1_s2s_sqwqe_to_lkey_inv_info t1_s2s_sqwqe_to_lkey_frpmr_info

metadata req_tx_bktrack_sqcb2_write_back_info_t t1_s2s_bktrack_sqcb2_write_back_info;
@pragma scratch_metadata
metadata req_tx_bktrack_sqcb2_write_back_info_t t1_s2s_bktrack_sqcb2_write_back_info_scr;

metadata req_tx_bktrack_sqcb1_write_back_info_t t1_s2s_bktrack_sqcb1_write_back_info;
@pragma scratch_metadata
metadata req_tx_bktrack_sqcb1_write_back_info_t t1_s2s_bktrack_sqcb1_write_back_info_scr;

metadata req_tx_sge_to_lkey_info_t t1_s2s_sge_to_lkey_info;
@pragma scratch_metadata
metadata req_tx_sge_to_lkey_info_t t1_s2s_sge_to_lkey_info_scr;

metadata req_tx_lkey_to_ptseg_info_t t1_s2s_lkey_to_ptseg_info;
@pragma scratch_metadata
metadata req_tx_lkey_to_ptseg_info_t t1_s2s_lkey_to_ptseg_info_scr;

metadata req_tx_sqwqe_to_lkey_inv_info_t t1_s2s_sqwqe_to_lkey_inv_info;
@pragma scratch_metadata
metadata req_tx_sqwqe_to_lkey_inv_info_t t1_s2s_sqwqe_to_lkey_inv_info_scr;

metadata req_tx_sqwqe_to_lkey_frpmr_info_t t1_s2s_sqwqe_to_lkey_frpmr_info;
@pragma scratch_metadata
metadata req_tx_sqwqe_to_lkey_frpmr_info_t t1_s2s_sqwqe_to_lkey_frpmr_info_scr;


//Table-2
@pragma pa_header_union ingress common_t2_s2s t2_s2s_wqe_to_sge_info t2_s2s_sqcb_write_back_info t2_s2s_sqcb_write_back_info_rd t2_s2s_sqcb_write_back_info_send_wr t2_s2s_frpmr_write_back_info

metadata req_tx_wqe_to_sge_info_t t2_s2s_wqe_to_sge_info;
@pragma scratch_metadata
metadata req_tx_wqe_to_sge_info_t t2_s2s_wqe_to_sge_info_scr;

metadata req_tx_sqcb_write_back_info_t t2_s2s_sqcb_write_back_info;
@pragma scratch_metadata
metadata req_tx_sqcb_write_back_info_t t2_s2s_sqcb_write_back_info_scr;

metadata req_tx_sqcb_write_back_info_rd_t t2_s2s_sqcb_write_back_info_rd;
@pragma scratch_metadata
metadata req_tx_sqcb_write_back_info_rd_t t2_s2s_sqcb_write_back_info_rd_scr;

metadata req_tx_sqcb_write_back_info_send_wr_t t2_s2s_sqcb_write_back_info_send_wr;
@pragma scratch_metadata
metadata req_tx_sqcb_write_back_info_send_wr_t t2_s2s_sqcb_write_back_info_send_wr_scr;

metadata req_tx_frpmr_lkey_to_wb_info_t t2_s2s_frpmr_write_back_info;
@pragma scratch_metadata
metadata req_tx_frpmr_lkey_to_wb_info_t t2_s2s_frpmr_write_back_info_scr;


//Table-3
@pragma pa_header_union ingress common_t3_s2s  t3_s2s_add_hdr_info t3_s2s_stats_info

metadata req_tx_add_hdr_info_t t3_s2s_add_hdr_info;
@pragma scratch_metadata
metadata req_tx_add_hdr_info_t t3_s2s_add_hdr_info_scr;

metadata req_tx_stats_info_t t3_s2s_stats_info;
@pragma scratch_metadata
metadata req_tx_stats_info_t t3_s2s_stats_info_scr;

/*
 * Stage 0 table 0 recirc action
 */
action req_tx_stage0_recirc_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_txdma_intr_scratch.qid, p4_txdma_intr.qid);
    modify_field(p4_txdma_intr_scratch.qtype, p4_txdma_intr.qtype);
    modify_field(p4_txdma_intr_scratch.qstate_addr, p4_txdma_intr.qstate_addr);

    // recirc header bits
    modify_field(rdma_recirc_scr.recirc_reason, rdma_recirc.recirc_reason);
    modify_field(rdma_recirc_scr.recirc_spec_cindex, rdma_recirc.recirc_spec_cindex);
}


action req_tx_bktrack_sqcb2_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_bt_info_scr.wqe_addr, to_s1_bt_info.wqe_addr);
    modify_field(to_s1_bt_info_scr.log_pmtu, to_s1_bt_info.log_pmtu);
    modify_field(to_s1_bt_info_scr.log_sq_page_size, to_s1_bt_info.log_sq_page_size);
    modify_field(to_s1_bt_info_scr.log_wqe_size, to_s1_bt_info.log_wqe_size);
    modify_field(to_s1_bt_info_scr.log_num_wqes, to_s1_bt_info.log_num_wqes);
    modify_field(to_s1_bt_info_scr.rexmit_psn, to_s1_bt_info.rexmit_psn);
    modify_field(to_s1_bt_info_scr.pad, to_s1_bt_info.pad);

    // stage to stage
    modify_field(t0_s2s_sqcb0_to_sqcb2_info_scr.sq_c_index, t0_s2s_sqcb0_to_sqcb2_info.sq_c_index);
    modify_field(t0_s2s_sqcb0_to_sqcb2_info_scr.sq_p_index, t0_s2s_sqcb0_to_sqcb2_info.sq_p_index);
    modify_field(t0_s2s_sqcb0_to_sqcb2_info_scr.in_progress, t0_s2s_sqcb0_to_sqcb2_info.in_progress);
    modify_field(t0_s2s_sqcb0_to_sqcb2_info_scr.bktrack_in_progress, t0_s2s_sqcb0_to_sqcb2_info.bktrack_in_progress);
    modify_field(t0_s2s_sqcb0_to_sqcb2_info_scr.current_sge_offset, t0_s2s_sqcb0_to_sqcb2_info.current_sge_offset);
    modify_field(t0_s2s_sqcb0_to_sqcb2_info_scr.current_sge_id, t0_s2s_sqcb0_to_sqcb2_info.current_sge_id);
    modify_field(t0_s2s_sqcb0_to_sqcb2_info_scr.num_sges, t0_s2s_sqcb0_to_sqcb2_info.num_sges);
    modify_field(t0_s2s_sqcb0_to_sqcb2_info_scr.update_credits, t0_s2s_sqcb0_to_sqcb2_info.update_credits);
    modify_field(t0_s2s_sqcb0_to_sqcb2_info_scr.bktrack, t0_s2s_sqcb0_to_sqcb2_info.bktrack);
    modify_field(t0_s2s_sqcb0_to_sqcb2_info_scr.pt_base_addr, t0_s2s_sqcb0_to_sqcb2_info.pt_base_addr);
    modify_field(t0_s2s_sqcb0_to_sqcb2_info_scr.op_type, t0_s2s_sqcb0_to_sqcb2_info.op_type);
    modify_field(t0_s2s_sqcb0_to_sqcb2_info_scr.sq_in_hbm, t0_s2s_sqcb0_to_sqcb2_info.sq_in_hbm);
    modify_field(t0_s2s_sqcb0_to_sqcb2_info_scr.skip_pt, t0_s2s_sqcb0_to_sqcb2_info.skip_pt);
    modify_field(t0_s2s_sqcb0_to_sqcb2_info_scr.spec_enable, t0_s2s_sqcb0_to_sqcb2_info.spec_enable);
    modify_field(t0_s2s_sqcb0_to_sqcb2_info_scr.pad, t0_s2s_sqcb0_to_sqcb2_info.pad);
}
action req_tx_timer_expiry_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
}

action req_tx_sq_drain_feedback_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
}

action req_tx_bktrack_sqcb1_write_back_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t1_s2s_bktrack_sqcb1_write_back_info_scr.tx_psn, t1_s2s_bktrack_sqcb1_write_back_info.tx_psn);
    modify_field(t1_s2s_bktrack_sqcb1_write_back_info_scr.ssn, t1_s2s_bktrack_sqcb1_write_back_info.ssn);
}

action req_tx_bktrack_sqpt_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s2_bt_info_scr.wqe_addr, to_s2_bt_info.wqe_addr);
    modify_field(to_s2_bt_info_scr.log_pmtu, to_s2_bt_info.log_pmtu);
    modify_field(to_s2_bt_info_scr.log_sq_page_size, to_s2_bt_info.log_sq_page_size);
    modify_field(to_s2_bt_info_scr.log_wqe_size, to_s2_bt_info.log_wqe_size);
    modify_field(to_s2_bt_info_scr.log_num_wqes, to_s2_bt_info.log_num_wqes);
    modify_field(to_s2_bt_info_scr.rexmit_psn, to_s2_bt_info.rexmit_psn);
    modify_field(to_s2_bt_info_scr.pad, to_s2_bt_info.pad);

    // stage to stage
    modify_field(t0_s2s_sq_bktrack_info_scr.sq_c_index, t0_s2s_sq_bktrack_info.sq_c_index);
    modify_field(t0_s2s_sq_bktrack_info_scr.in_progress, t0_s2s_sq_bktrack_info.in_progress);
    modify_field(t0_s2s_sq_bktrack_info_scr.current_sge_offset, t0_s2s_sq_bktrack_info.current_sge_offset);
    modify_field(t0_s2s_sq_bktrack_info_scr.current_sge_id, t0_s2s_sq_bktrack_info.current_sge_id);
    modify_field(t0_s2s_sq_bktrack_info_scr.num_sges, t0_s2s_sq_bktrack_info.num_sges);
    modify_field(t0_s2s_sq_bktrack_info_scr.tx_psn, t0_s2s_sq_bktrack_info.tx_psn);
    modify_field(t0_s2s_sq_bktrack_info_scr.ssn, t0_s2s_sq_bktrack_info.ssn);
    modify_field(t0_s2s_sq_bktrack_info_scr.sq_p_index_or_imm_data1_or_inv_key1, t0_s2s_sq_bktrack_info.sq_p_index_or_imm_data1_or_inv_key1);
    modify_field(t0_s2s_sq_bktrack_info_scr.imm_data2_or_inv_key2, t0_s2s_sq_bktrack_info.imm_data2_or_inv_key2);
    modify_field(t0_s2s_sq_bktrack_info_scr.spec_enable, t0_s2s_sq_bktrack_info.spec_enable);
    modify_field(t0_s2s_sq_bktrack_info_scr.pad, t0_s2s_sq_bktrack_info.pad);
}

action req_tx_bktrack_sqsge_process_s3 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s3_bt_info_scr.wqe_addr, to_s3_bt_info.wqe_addr);
    modify_field(to_s3_bt_info_scr.log_pmtu, to_s3_bt_info.log_pmtu);
    modify_field(to_s3_bt_info_scr.log_sq_page_size, to_s3_bt_info.log_sq_page_size);
    modify_field(to_s3_bt_info_scr.log_wqe_size, to_s3_bt_info.log_wqe_size);
    modify_field(to_s3_bt_info_scr.log_num_wqes, to_s3_bt_info.log_num_wqes);
    modify_field(to_s3_bt_info_scr.rexmit_psn, to_s3_bt_info.rexmit_psn);
    modify_field(to_s3_bt_info_scr.pad, to_s3_bt_info.pad);

    // stage to stage
    modify_field(t0_s2s_sq_bktrack_info_scr.sq_c_index, t0_s2s_sq_bktrack_info.sq_c_index);
    modify_field(t0_s2s_sq_bktrack_info_scr.in_progress, t0_s2s_sq_bktrack_info.in_progress);
    modify_field(t0_s2s_sq_bktrack_info_scr.current_sge_offset, t0_s2s_sq_bktrack_info.current_sge_offset);
    modify_field(t0_s2s_sq_bktrack_info_scr.current_sge_id, t0_s2s_sq_bktrack_info.current_sge_id);
    modify_field(t0_s2s_sq_bktrack_info_scr.num_sges, t0_s2s_sq_bktrack_info.num_sges);
    modify_field(t0_s2s_sq_bktrack_info_scr.tx_psn, t0_s2s_sq_bktrack_info.tx_psn);
    modify_field(t0_s2s_sq_bktrack_info_scr.ssn, t0_s2s_sq_bktrack_info.ssn);
    modify_field(t0_s2s_sq_bktrack_info_scr.op_type, t0_s2s_sq_bktrack_info.op_type);
    modify_field(t0_s2s_sq_bktrack_info_scr.sq_p_index_or_imm_data1_or_inv_key1, t0_s2s_sq_bktrack_info.sq_p_index_or_imm_data1_or_inv_key1);
    modify_field(t0_s2s_sq_bktrack_info_scr.imm_data2_or_inv_key2, t0_s2s_sq_bktrack_info.imm_data2_or_inv_key2);
    modify_field(t0_s2s_sq_bktrack_info_scr.spec_enable, t0_s2s_sq_bktrack_info.spec_enable);
    modify_field(t0_s2s_sq_bktrack_info_scr.pad, t0_s2s_sq_bktrack_info.pad);

}

action req_tx_bktrack_sqsge_process_s4 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_bt_info_scr.wqe_addr, to_s4_bt_info.wqe_addr);
    modify_field(to_s4_bt_info_scr.log_pmtu, to_s4_bt_info.log_pmtu);
    modify_field(to_s4_bt_info_scr.log_sq_page_size, to_s4_bt_info.log_sq_page_size);
    modify_field(to_s4_bt_info_scr.log_wqe_size, to_s4_bt_info.log_wqe_size);
    modify_field(to_s4_bt_info_scr.log_num_wqes, to_s4_bt_info.log_num_wqes);
    modify_field(to_s4_bt_info_scr.rexmit_psn, to_s4_bt_info.rexmit_psn);
    modify_field(to_s4_bt_info_scr.pad, to_s4_bt_info.pad);

    // stage to stage
    modify_field(t0_s2s_sq_bktrack_info_scr.sq_c_index, t0_s2s_sq_bktrack_info.sq_c_index);
    modify_field(t0_s2s_sq_bktrack_info_scr.in_progress, t0_s2s_sq_bktrack_info.in_progress);
    modify_field(t0_s2s_sq_bktrack_info_scr.current_sge_offset, t0_s2s_sq_bktrack_info.current_sge_offset);
    modify_field(t0_s2s_sq_bktrack_info_scr.current_sge_id, t0_s2s_sq_bktrack_info.current_sge_id);
    modify_field(t0_s2s_sq_bktrack_info_scr.num_sges, t0_s2s_sq_bktrack_info.num_sges);
    modify_field(t0_s2s_sq_bktrack_info_scr.tx_psn, t0_s2s_sq_bktrack_info.tx_psn);
    modify_field(t0_s2s_sq_bktrack_info_scr.ssn, t0_s2s_sq_bktrack_info.ssn);
    modify_field(t0_s2s_sq_bktrack_info_scr.op_type, t0_s2s_sq_bktrack_info.op_type);
    modify_field(t0_s2s_sq_bktrack_info_scr.sq_p_index_or_imm_data1_or_inv_key1, t0_s2s_sq_bktrack_info.sq_p_index_or_imm_data1_or_inv_key1);
    modify_field(t0_s2s_sq_bktrack_info_scr.imm_data2_or_inv_key2, t0_s2s_sq_bktrack_info.imm_data2_or_inv_key2);
    modify_field(t0_s2s_sq_bktrack_info_scr.spec_enable, t0_s2s_sq_bktrack_info.spec_enable);
    modify_field(t0_s2s_sq_bktrack_info_scr.pad, t0_s2s_sq_bktrack_info.pad);

}

action req_tx_bktrack_sqsge_process_s5 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s5_bt_info_scr.wqe_addr, to_s5_bt_info.wqe_addr);
    modify_field(to_s5_bt_info_scr.log_pmtu, to_s5_bt_info.log_pmtu);
    modify_field(to_s5_bt_info_scr.log_sq_page_size, to_s5_bt_info.log_sq_page_size);
    modify_field(to_s5_bt_info_scr.log_wqe_size, to_s5_bt_info.log_wqe_size);
    modify_field(to_s5_bt_info_scr.log_num_wqes, to_s5_bt_info.log_num_wqes);
    modify_field(to_s5_bt_info_scr.rexmit_psn, to_s5_bt_info.rexmit_psn);
    modify_field(to_s5_bt_info_scr.pad, to_s5_bt_info.pad);

    // stage to stage
    modify_field(t0_s2s_sq_bktrack_info_scr.sq_c_index, t0_s2s_sq_bktrack_info.sq_c_index);
    modify_field(t0_s2s_sq_bktrack_info_scr.in_progress, t0_s2s_sq_bktrack_info.in_progress);
    modify_field(t0_s2s_sq_bktrack_info_scr.current_sge_offset, t0_s2s_sq_bktrack_info.current_sge_offset);
    modify_field(t0_s2s_sq_bktrack_info_scr.current_sge_id, t0_s2s_sq_bktrack_info.current_sge_id);
    modify_field(t0_s2s_sq_bktrack_info_scr.num_sges, t0_s2s_sq_bktrack_info.num_sges);
    modify_field(t0_s2s_sq_bktrack_info_scr.tx_psn, t0_s2s_sq_bktrack_info.tx_psn);
    modify_field(t0_s2s_sq_bktrack_info_scr.ssn, t0_s2s_sq_bktrack_info.ssn);
    modify_field(t0_s2s_sq_bktrack_info_scr.op_type, t0_s2s_sq_bktrack_info.op_type);
    modify_field(t0_s2s_sq_bktrack_info_scr.sq_p_index_or_imm_data1_or_inv_key1, t0_s2s_sq_bktrack_info.sq_p_index_or_imm_data1_or_inv_key1);
    modify_field(t0_s2s_sq_bktrack_info_scr.imm_data2_or_inv_key2, t0_s2s_sq_bktrack_info.imm_data2_or_inv_key2);
    modify_field(t0_s2s_sq_bktrack_info_scr.spec_enable, t0_s2s_sq_bktrack_info.spec_enable);
    modify_field(t0_s2s_sq_bktrack_info_scr.pad, t0_s2s_sq_bktrack_info.pad);

}

action req_tx_bktrack_sqsge_process_s6 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s6_bt_info_scr.wqe_addr, to_s6_bt_info.wqe_addr);
    modify_field(to_s6_bt_info_scr.log_pmtu, to_s6_bt_info.log_pmtu);
    modify_field(to_s6_bt_info_scr.log_sq_page_size, to_s6_bt_info.log_sq_page_size);
    modify_field(to_s6_bt_info_scr.log_wqe_size, to_s6_bt_info.log_wqe_size);
    modify_field(to_s6_bt_info_scr.log_num_wqes, to_s6_bt_info.log_num_wqes);
    modify_field(to_s6_bt_info_scr.rexmit_psn, to_s6_bt_info.rexmit_psn);
    modify_field(to_s6_bt_info_scr.pad, to_s6_bt_info.pad);

    // stage to stage
    modify_field(t0_s2s_sq_bktrack_info_scr.sq_c_index, t0_s2s_sq_bktrack_info.sq_c_index);
    modify_field(t0_s2s_sq_bktrack_info_scr.in_progress, t0_s2s_sq_bktrack_info.in_progress);
    modify_field(t0_s2s_sq_bktrack_info_scr.current_sge_offset, t0_s2s_sq_bktrack_info.current_sge_offset);
    modify_field(t0_s2s_sq_bktrack_info_scr.current_sge_id, t0_s2s_sq_bktrack_info.current_sge_id);
    modify_field(t0_s2s_sq_bktrack_info_scr.num_sges, t0_s2s_sq_bktrack_info.num_sges);
    modify_field(t0_s2s_sq_bktrack_info_scr.tx_psn, t0_s2s_sq_bktrack_info.tx_psn);
    modify_field(t0_s2s_sq_bktrack_info_scr.ssn, t0_s2s_sq_bktrack_info.ssn);
    modify_field(t0_s2s_sq_bktrack_info_scr.op_type, t0_s2s_sq_bktrack_info.op_type);
    modify_field(t0_s2s_sq_bktrack_info_scr.sq_p_index_or_imm_data1_or_inv_key1, t0_s2s_sq_bktrack_info.sq_p_index_or_imm_data1_or_inv_key1);
    modify_field(t0_s2s_sq_bktrack_info_scr.imm_data2_or_inv_key2, t0_s2s_sq_bktrack_info.imm_data2_or_inv_key2);
    modify_field(t0_s2s_sq_bktrack_info_scr.spec_enable, t0_s2s_sq_bktrack_info.spec_enable);
    modify_field(t0_s2s_sq_bktrack_info_scr.pad, t0_s2s_sq_bktrack_info.pad);

}
action req_tx_bktrack_sqwqe_process_s2 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s2_bt_info_scr.wqe_addr, to_s2_bt_info.wqe_addr);
    modify_field(to_s2_bt_info_scr.log_pmtu, to_s2_bt_info.log_pmtu);
    modify_field(to_s2_bt_info_scr.log_sq_page_size, to_s2_bt_info.log_sq_page_size);
    modify_field(to_s2_bt_info_scr.log_wqe_size, to_s2_bt_info.log_wqe_size);
    modify_field(to_s2_bt_info_scr.log_num_wqes, to_s2_bt_info.log_num_wqes);
    modify_field(to_s2_bt_info_scr.rexmit_psn, to_s2_bt_info.rexmit_psn);
    modify_field(to_s2_bt_info_scr.pad, to_s2_bt_info.pad);

    // stage to stage
    modify_field(t0_s2s_sq_bktrack_info_scr.sq_c_index, t0_s2s_sq_bktrack_info.sq_c_index);
    modify_field(t0_s2s_sq_bktrack_info_scr.in_progress, t0_s2s_sq_bktrack_info.in_progress);
    modify_field(t0_s2s_sq_bktrack_info_scr.current_sge_offset, t0_s2s_sq_bktrack_info.current_sge_offset);
    modify_field(t0_s2s_sq_bktrack_info_scr.current_sge_id, t0_s2s_sq_bktrack_info.current_sge_id);
    modify_field(t0_s2s_sq_bktrack_info_scr.num_sges, t0_s2s_sq_bktrack_info.num_sges);
    modify_field(t0_s2s_sq_bktrack_info_scr.tx_psn, t0_s2s_sq_bktrack_info.tx_psn);
    modify_field(t0_s2s_sq_bktrack_info_scr.ssn, t0_s2s_sq_bktrack_info.ssn);
    modify_field(t0_s2s_sq_bktrack_info_scr.op_type, t0_s2s_sq_bktrack_info.op_type);
    modify_field(t0_s2s_sq_bktrack_info_scr.sq_p_index_or_imm_data1_or_inv_key1, t0_s2s_sq_bktrack_info.sq_p_index_or_imm_data1_or_inv_key1);
    modify_field(t0_s2s_sq_bktrack_info_scr.imm_data2_or_inv_key2, t0_s2s_sq_bktrack_info.imm_data2_or_inv_key2);
    modify_field(t0_s2s_sq_bktrack_info_scr.spec_enable, t0_s2s_sq_bktrack_info.spec_enable);
    modify_field(t0_s2s_sq_bktrack_info_scr.pad, t0_s2s_sq_bktrack_info.pad);

}

action req_tx_bktrack_sqwqe_process_s3 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s3_bt_info_scr.wqe_addr, to_s3_bt_info.wqe_addr);
    modify_field(to_s3_bt_info_scr.log_pmtu, to_s3_bt_info.log_pmtu);
    modify_field(to_s3_bt_info_scr.log_sq_page_size, to_s3_bt_info.log_sq_page_size);
    modify_field(to_s3_bt_info_scr.log_wqe_size, to_s3_bt_info.log_wqe_size);
    modify_field(to_s3_bt_info_scr.log_num_wqes, to_s3_bt_info.log_num_wqes);
    modify_field(to_s3_bt_info_scr.rexmit_psn, to_s3_bt_info.rexmit_psn);
    modify_field(to_s3_bt_info_scr.pad, to_s3_bt_info.pad);

    // stage to stage
    modify_field(t0_s2s_sq_bktrack_info_scr.sq_c_index, t0_s2s_sq_bktrack_info.sq_c_index);
    modify_field(t0_s2s_sq_bktrack_info_scr.in_progress, t0_s2s_sq_bktrack_info.in_progress);
    modify_field(t0_s2s_sq_bktrack_info_scr.current_sge_offset, t0_s2s_sq_bktrack_info.current_sge_offset);
    modify_field(t0_s2s_sq_bktrack_info_scr.current_sge_id, t0_s2s_sq_bktrack_info.current_sge_id);
    modify_field(t0_s2s_sq_bktrack_info_scr.num_sges, t0_s2s_sq_bktrack_info.num_sges);
    modify_field(t0_s2s_sq_bktrack_info_scr.tx_psn, t0_s2s_sq_bktrack_info.tx_psn);
    modify_field(t0_s2s_sq_bktrack_info_scr.ssn, t0_s2s_sq_bktrack_info.ssn);
    modify_field(t0_s2s_sq_bktrack_info_scr.op_type, t0_s2s_sq_bktrack_info.op_type);
    modify_field(t0_s2s_sq_bktrack_info_scr.sq_p_index_or_imm_data1_or_inv_key1, t0_s2s_sq_bktrack_info.sq_p_index_or_imm_data1_or_inv_key1);
    modify_field(t0_s2s_sq_bktrack_info_scr.imm_data2_or_inv_key2, t0_s2s_sq_bktrack_info.imm_data2_or_inv_key2);
    modify_field(t0_s2s_sq_bktrack_info_scr.spec_enable, t0_s2s_sq_bktrack_info.spec_enable);
    modify_field(t0_s2s_sq_bktrack_info_scr.pad, t0_s2s_sq_bktrack_info.pad);
}

action req_tx_bktrack_sqwqe_process_s4 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_bt_info_scr.wqe_addr, to_s4_bt_info.wqe_addr);
    modify_field(to_s4_bt_info_scr.log_pmtu, to_s4_bt_info.log_pmtu);
    modify_field(to_s4_bt_info_scr.log_sq_page_size, to_s4_bt_info.log_sq_page_size);
    modify_field(to_s4_bt_info_scr.log_wqe_size, to_s4_bt_info.log_wqe_size);
    modify_field(to_s4_bt_info_scr.log_num_wqes, to_s4_bt_info.log_num_wqes);
    modify_field(to_s4_bt_info_scr.rexmit_psn, to_s4_bt_info.rexmit_psn);
    modify_field(to_s4_bt_info_scr.pad, to_s4_bt_info.pad);

    // stage to stage
    modify_field(t0_s2s_sq_bktrack_info_scr.sq_c_index, t0_s2s_sq_bktrack_info.sq_c_index);
    modify_field(t0_s2s_sq_bktrack_info_scr.in_progress, t0_s2s_sq_bktrack_info.in_progress);
    modify_field(t0_s2s_sq_bktrack_info_scr.current_sge_offset, t0_s2s_sq_bktrack_info.current_sge_offset);
    modify_field(t0_s2s_sq_bktrack_info_scr.current_sge_id, t0_s2s_sq_bktrack_info.current_sge_id);
    modify_field(t0_s2s_sq_bktrack_info_scr.num_sges, t0_s2s_sq_bktrack_info.num_sges);
    modify_field(t0_s2s_sq_bktrack_info_scr.tx_psn, t0_s2s_sq_bktrack_info.tx_psn);
    modify_field(t0_s2s_sq_bktrack_info_scr.ssn, t0_s2s_sq_bktrack_info.ssn);
    modify_field(t0_s2s_sq_bktrack_info_scr.op_type, t0_s2s_sq_bktrack_info.op_type);
    modify_field(t0_s2s_sq_bktrack_info_scr.sq_p_index_or_imm_data1_or_inv_key1, t0_s2s_sq_bktrack_info.sq_p_index_or_imm_data1_or_inv_key1);
    modify_field(t0_s2s_sq_bktrack_info_scr.imm_data2_or_inv_key2, t0_s2s_sq_bktrack_info.imm_data2_or_inv_key2);
    modify_field(t0_s2s_sq_bktrack_info_scr.spec_enable, t0_s2s_sq_bktrack_info.spec_enable);
    modify_field(t0_s2s_sq_bktrack_info_scr.pad, t0_s2s_sq_bktrack_info.pad);
}


action req_tx_bktrack_sqwqe_process_s5 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s5_bt_info_scr.wqe_addr, to_s5_bt_info.wqe_addr);
    modify_field(to_s5_bt_info_scr.log_pmtu, to_s5_bt_info.log_pmtu);
    modify_field(to_s5_bt_info_scr.log_sq_page_size, to_s5_bt_info.log_sq_page_size);
    modify_field(to_s5_bt_info_scr.log_wqe_size, to_s5_bt_info.log_wqe_size);
    modify_field(to_s5_bt_info_scr.log_num_wqes, to_s5_bt_info.log_num_wqes);
    modify_field(to_s5_bt_info_scr.rexmit_psn, to_s5_bt_info.rexmit_psn);
    modify_field(to_s5_bt_info_scr.pad, to_s5_bt_info.pad);

    // stage to stage
    modify_field(t0_s2s_sq_bktrack_info_scr.sq_c_index, t0_s2s_sq_bktrack_info.sq_c_index);
    modify_field(t0_s2s_sq_bktrack_info_scr.in_progress, t0_s2s_sq_bktrack_info.in_progress);
    modify_field(t0_s2s_sq_bktrack_info_scr.current_sge_offset, t0_s2s_sq_bktrack_info.current_sge_offset);
    modify_field(t0_s2s_sq_bktrack_info_scr.current_sge_id, t0_s2s_sq_bktrack_info.current_sge_id);
    modify_field(t0_s2s_sq_bktrack_info_scr.num_sges, t0_s2s_sq_bktrack_info.num_sges);
    modify_field(t0_s2s_sq_bktrack_info_scr.tx_psn, t0_s2s_sq_bktrack_info.tx_psn);
    modify_field(t0_s2s_sq_bktrack_info_scr.ssn, t0_s2s_sq_bktrack_info.ssn);
    modify_field(t0_s2s_sq_bktrack_info_scr.op_type, t0_s2s_sq_bktrack_info.op_type);
    modify_field(t0_s2s_sq_bktrack_info_scr.sq_p_index_or_imm_data1_or_inv_key1, t0_s2s_sq_bktrack_info.sq_p_index_or_imm_data1_or_inv_key1);
    modify_field(t0_s2s_sq_bktrack_info_scr.imm_data2_or_inv_key2, t0_s2s_sq_bktrack_info.imm_data2_or_inv_key2);
    modify_field(t0_s2s_sq_bktrack_info_scr.spec_enable, t0_s2s_sq_bktrack_info.spec_enable);
    modify_field(t0_s2s_sq_bktrack_info_scr.pad, t0_s2s_sq_bktrack_info.pad);
}


action req_tx_bktrack_sqwqe_process_s6 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s6_bt_info_scr.wqe_addr, to_s6_bt_info.wqe_addr);
    modify_field(to_s6_bt_info_scr.log_pmtu, to_s6_bt_info.log_pmtu);
    modify_field(to_s6_bt_info_scr.log_sq_page_size, to_s6_bt_info.log_sq_page_size);
    modify_field(to_s6_bt_info_scr.log_wqe_size, to_s6_bt_info.log_wqe_size);
    modify_field(to_s6_bt_info_scr.log_num_wqes, to_s6_bt_info.log_num_wqes);
    modify_field(to_s6_bt_info_scr.rexmit_psn, to_s6_bt_info.rexmit_psn);
    modify_field(to_s6_bt_info_scr.pad, to_s6_bt_info.pad);

    // stage to stage
    modify_field(t0_s2s_sq_bktrack_info_scr.sq_c_index, t0_s2s_sq_bktrack_info.sq_c_index);
    modify_field(t0_s2s_sq_bktrack_info_scr.in_progress, t0_s2s_sq_bktrack_info.in_progress);
    modify_field(t0_s2s_sq_bktrack_info_scr.current_sge_offset, t0_s2s_sq_bktrack_info.current_sge_offset);
    modify_field(t0_s2s_sq_bktrack_info_scr.current_sge_id, t0_s2s_sq_bktrack_info.current_sge_id);
    modify_field(t0_s2s_sq_bktrack_info_scr.num_sges, t0_s2s_sq_bktrack_info.num_sges);
    modify_field(t0_s2s_sq_bktrack_info_scr.tx_psn, t0_s2s_sq_bktrack_info.tx_psn);
    modify_field(t0_s2s_sq_bktrack_info_scr.ssn, t0_s2s_sq_bktrack_info.ssn);
    modify_field(t0_s2s_sq_bktrack_info_scr.op_type, t0_s2s_sq_bktrack_info.op_type);
    modify_field(t0_s2s_sq_bktrack_info_scr.sq_p_index_or_imm_data1_or_inv_key1, t0_s2s_sq_bktrack_info.sq_p_index_or_imm_data1_or_inv_key1);
    modify_field(t0_s2s_sq_bktrack_info_scr.imm_data2_or_inv_key2, t0_s2s_sq_bktrack_info.imm_data2_or_inv_key2);
    modify_field(t0_s2s_sq_bktrack_info_scr.spec_enable, t0_s2s_sq_bktrack_info.spec_enable);
    modify_field(t0_s2s_sq_bktrack_info_scr.pad, t0_s2s_sq_bktrack_info.pad);
}

action req_tx_bktrack_write_back_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s7_bt_wb_info_scr.wqe_addr, to_s7_bt_wb_info.wqe_addr);
    modify_field(to_s7_bt_wb_info_scr.wqe_start_psn, to_s7_bt_wb_info.wqe_start_psn);

    // stage to stage
    modify_field(t0_s2s_sqcb_write_back_info_scr.hdr_template_inline, t0_s2s_sqcb_write_back_info.hdr_template_inline);
    modify_field(t0_s2s_sqcb_write_back_info_scr.busy, t0_s2s_sqcb_write_back_info.busy);
    modify_field(t0_s2s_sqcb_write_back_info_scr.in_progress, t0_s2s_sqcb_write_back_info.in_progress);
    modify_field(t0_s2s_sqcb_write_back_info_scr.bktrack_in_progress, t0_s2s_sqcb_write_back_info.bktrack_in_progress);
    modify_field(t0_s2s_sqcb_write_back_info_scr.op_type, t0_s2s_sqcb_write_back_info.op_type);
    modify_field(t0_s2s_sqcb_write_back_info_scr.first, t0_s2s_sqcb_write_back_info.first);
    modify_field(t0_s2s_sqcb_write_back_info_scr.last_pkt, t0_s2s_sqcb_write_back_info.last_pkt);
    modify_field(t0_s2s_sqcb_write_back_info_scr.set_fence, t0_s2s_sqcb_write_back_info.set_fence);
    modify_field(t0_s2s_sqcb_write_back_info_scr.set_li_fence, t0_s2s_sqcb_write_back_info.set_li_fence);
    modify_field(t0_s2s_sqcb_write_back_info_scr.empty_rrq_bktrack, t0_s2s_sqcb_write_back_info.empty_rrq_bktrack);
    modify_field(t0_s2s_sqcb_write_back_info_scr.current_sge_offset, t0_s2s_sqcb_write_back_info.current_sge_offset);
    modify_field(t0_s2s_sqcb_write_back_info_scr.current_sge_id, t0_s2s_sqcb_write_back_info.current_sge_id);
    modify_field(t0_s2s_sqcb_write_back_info_scr.num_sges, t0_s2s_sqcb_write_back_info.num_sges);
    modify_field(t0_s2s_sqcb_write_back_info_scr.sq_c_index, t0_s2s_sqcb_write_back_info.sq_c_index);
    modify_field(t0_s2s_sqcb_write_back_info_scr.ah_size, t0_s2s_sqcb_write_back_info.ah_size);
    modify_field(t0_s2s_sqcb_write_back_info_scr.poll_in_progress, t0_s2s_sqcb_write_back_info.poll_in_progress);
    modify_field(t0_s2s_sqcb_write_back_info_scr.color, t0_s2s_sqcb_write_back_info.color);
    modify_field(t0_s2s_sqcb_write_back_info_scr.poll_failed, t0_s2s_sqcb_write_back_info.poll_failed);
    modify_field(t0_s2s_sqcb_write_back_info_scr.rate_enforce_failed, t0_s2s_sqcb_write_back_info.rate_enforce_failed);
    modify_field(t0_s2s_sqcb_write_back_info_scr.non_packet_wqe, t0_s2s_sqcb_write_back_info.non_packet_wqe);
    modify_field(t0_s2s_sqcb_write_back_info_scr.credit_check_failed, t0_s2s_sqcb_write_back_info.credit_check_failed);
    modify_field(t0_s2s_sqcb_write_back_info_scr.drop_phv, t0_s2s_sqcb_write_back_info.drop_phv);
    modify_field(t0_s2s_sqcb_write_back_info_scr.drop_bktrack, t0_s2s_sqcb_write_back_info.drop_bktrack);
    modify_field(t0_s2s_sqcb_write_back_info_scr.rsvd, t0_s2s_sqcb_write_back_info.rsvd);
    modify_field(t0_s2s_sqcb_write_back_info_scr.op_info, t0_s2s_sqcb_write_back_info.op_info);

}

action req_tx_bktrack_sqcb2_write_back_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s7_bt_wb_info_scr.wqe_addr, to_s7_bt_wb_info.wqe_addr);
    modify_field(to_s7_bt_wb_info_scr.wqe_start_psn, to_s7_bt_wb_info.wqe_start_psn);

    // stage to stage
    modify_field(t1_s2s_bktrack_sqcb2_write_back_info_scr.tx_psn, t1_s2s_bktrack_sqcb2_write_back_info.tx_psn);
    modify_field(t1_s2s_bktrack_sqcb2_write_back_info_scr.ssn, t1_s2s_bktrack_sqcb2_write_back_info.ssn);
    modify_field(t1_s2s_bktrack_sqcb2_write_back_info_scr.imm_data_or_inv_key, t1_s2s_bktrack_sqcb2_write_back_info.imm_data_or_inv_key);
    modify_field(t1_s2s_bktrack_sqcb2_write_back_info_scr.pad1, t1_s2s_bktrack_sqcb2_write_back_info.pad1);
    modify_field(t1_s2s_bktrack_sqcb2_write_back_info_scr.op_type, t1_s2s_bktrack_sqcb2_write_back_info.op_type);
    modify_field(t1_s2s_bktrack_sqcb2_write_back_info_scr.sq_cindex, t1_s2s_bktrack_sqcb2_write_back_info.sq_cindex);
    modify_field(t1_s2s_bktrack_sqcb2_write_back_info_scr.in_progress, t1_s2s_bktrack_sqcb2_write_back_info.in_progress);
    modify_field(t1_s2s_bktrack_sqcb2_write_back_info_scr.bktrack_in_progress, t1_s2s_bktrack_sqcb2_write_back_info.bktrack_in_progress);
    modify_field(t1_s2s_bktrack_sqcb2_write_back_info_scr.msg_psn, t1_s2s_bktrack_sqcb2_write_back_info.msg_psn);
    modify_field(t1_s2s_bktrack_sqcb2_write_back_info_scr.pad, t1_s2s_bktrack_sqcb2_write_back_info.pad);

}
action req_tx_dcqcn_cnp_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage

}

action req_tx_sqcb2_cnp_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage

}

action req_tx_sqlkey_process_t0 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t0_s2s_sge_to_lkey_info_scr.sge_va, t0_s2s_sge_to_lkey_info.sge_va);
    modify_field(t0_s2s_sge_to_lkey_info_scr.sge_bytes, t0_s2s_sge_to_lkey_info.sge_bytes);
    modify_field(t0_s2s_sge_to_lkey_info_scr.dma_cmd_start_index, t0_s2s_sge_to_lkey_info.dma_cmd_start_index);
    modify_field(t0_s2s_sge_to_lkey_info_scr.sge_index, t0_s2s_sge_to_lkey_info.sge_index);
    modify_field(t0_s2s_sge_to_lkey_info_scr.lkey_invalidate, t0_s2s_sge_to_lkey_info.lkey_invalidate);
    modify_field(t0_s2s_sge_to_lkey_info_scr.rsvd_key_err, t0_s2s_sge_to_lkey_info.rsvd_key_err);
    modify_field(t0_s2s_sge_to_lkey_info_scr.pad, t0_s2s_sge_to_lkey_info.pad);

}

action req_tx_sqlkey_process_t1 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t1_s2s_sge_to_lkey_info_scr.sge_va, t1_s2s_sge_to_lkey_info.sge_va);
    modify_field(t1_s2s_sge_to_lkey_info_scr.sge_bytes, t1_s2s_sge_to_lkey_info.sge_bytes);
    modify_field(t1_s2s_sge_to_lkey_info_scr.dma_cmd_start_index, t1_s2s_sge_to_lkey_info.dma_cmd_start_index);
    modify_field(t1_s2s_sge_to_lkey_info_scr.sge_index, t1_s2s_sge_to_lkey_info.sge_index);
    modify_field(t1_s2s_sge_to_lkey_info_scr.lkey_invalidate, t1_s2s_sge_to_lkey_info.lkey_invalidate);
    modify_field(t1_s2s_sge_to_lkey_info_scr.rsvd_key_err, t1_s2s_sge_to_lkey_info.rsvd_key_err);
    modify_field(t1_s2s_sge_to_lkey_info_scr.pad, t1_s2s_sge_to_lkey_info.pad);

}

action req_tx_sqlkey_recirc_process_t0 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_dcqcn_bind_mw_info_scr.header_template_addr_or_pd, to_s1_dcqcn_bind_mw_info.header_template_addr_or_pd);
    modify_field(to_s1_dcqcn_bind_mw_info_scr.congestion_mgmt_type, to_s1_dcqcn_bind_mw_info.congestion_mgmt_type);


    // stage to stage
    modify_field(t0_s2s_sge_to_lkey_info_scr.sge_va, t0_s2s_sge_to_lkey_info.sge_va);
    modify_field(t0_s2s_sge_to_lkey_info_scr.sge_bytes, t0_s2s_sge_to_lkey_info.sge_bytes);
    modify_field(t0_s2s_sge_to_lkey_info_scr.dma_cmd_start_index, t0_s2s_sge_to_lkey_info.dma_cmd_start_index);
    modify_field(t0_s2s_sge_to_lkey_info_scr.sge_index, t0_s2s_sge_to_lkey_info.sge_index);
    modify_field(t0_s2s_sge_to_lkey_info_scr.lkey_invalidate, t0_s2s_sge_to_lkey_info.lkey_invalidate);
    modify_field(t0_s2s_sge_to_lkey_info_scr.rsvd_key_err, t0_s2s_sge_to_lkey_info.rsvd_key_err);
    modify_field(t0_s2s_sge_to_lkey_info_scr.pad, t0_s2s_sge_to_lkey_info.pad);

}

action req_tx_sqlkey_invalidate_process_s3 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s3_sqsge_info_scr.header_template_addr, to_s3_sqsge_info.header_template_addr);

    // stage to stage
    modify_field(t0_s2s_sqwqe_to_lkey_inv_info_scr.sge_index, t0_s2s_sqwqe_to_lkey_inv_info.sge_index);
    modify_field(t0_s2s_sqwqe_to_lkey_inv_info_scr.set_li_fence, t0_s2s_sqwqe_to_lkey_inv_info.set_li_fence);
    modify_field(t0_s2s_sqwqe_to_lkey_inv_info_scr.pad, t0_s2s_sqwqe_to_lkey_inv_info.pad);

}

action req_tx_sqlkey_invalidate_process_t0 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_dcqcn_bind_mw_info_scr.header_template_addr_or_pd, to_s4_dcqcn_bind_mw_info.header_template_addr_or_pd);
    modify_field(to_s4_dcqcn_bind_mw_info_scr.congestion_mgmt_type, to_s4_dcqcn_bind_mw_info.congestion_mgmt_type);

    // stage to stage
    modify_field(t0_s2s_sqwqe_to_lkey_inv_info_scr.sge_index, t0_s2s_sqwqe_to_lkey_inv_info.sge_index);
    modify_field(t0_s2s_sqwqe_to_lkey_inv_info_scr.set_li_fence, t0_s2s_sqwqe_to_lkey_inv_info.set_li_fence);
    modify_field(t0_s2s_sqwqe_to_lkey_inv_info_scr.pad, t0_s2s_sqwqe_to_lkey_inv_info.pad);

}

action req_tx_sqlkey_invalidate_process_t1 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t1_s2s_sqwqe_to_lkey_inv_info_scr.sge_index, t1_s2s_sqwqe_to_lkey_inv_info.sge_index);
    modify_field(t1_s2s_sqwqe_to_lkey_inv_info_scr.set_li_fence, t1_s2s_sqwqe_to_lkey_inv_info.set_li_fence);
    modify_field(t1_s2s_sqwqe_to_lkey_inv_info_scr.pad, t1_s2s_sqwqe_to_lkey_inv_info.pad);

}

action req_tx_frpmr_sqlkey_process_t0 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_frpmr_sqlkey_info_scr.header_template_addr_or_pd, to_s4_frpmr_sqlkey_info.header_template_addr_or_pd);
    modify_field(to_s4_frpmr_sqlkey_info_scr.congestion_mgmt_type, to_s4_frpmr_sqlkey_info.congestion_mgmt_type);
    modify_field(to_s4_frpmr_sqlkey_info_scr.len, to_s4_frpmr_sqlkey_info.len);
    modify_field(to_s4_frpmr_sqlkey_info_scr.rsvd, to_s4_frpmr_sqlkey_info.rsvd);

     //stage to stage
    modify_field(t0_s2s_sqwqe_to_lkey_frpmr_info_scr.sge_index, t0_s2s_sqwqe_to_lkey_frpmr_info.sge_index);
    modify_field(t0_s2s_sqwqe_to_lkey_frpmr_info_scr.fast_reg_rsvd_lkey_enable, t0_s2s_sqwqe_to_lkey_frpmr_info.fast_reg_rsvd_lkey_enable);
    modify_field(t0_s2s_sqwqe_to_lkey_frpmr_info_scr.zbva, t0_s2s_sqwqe_to_lkey_frpmr_info.zbva);
    modify_field(t0_s2s_sqwqe_to_lkey_frpmr_info_scr.lkey_state_update, t0_s2s_sqwqe_to_lkey_frpmr_info.lkey_state_update);
    modify_field(t0_s2s_sqwqe_to_lkey_frpmr_info_scr.zbva, t0_s2s_sqwqe_to_lkey_frpmr_info.zbva);
    modify_field(t0_s2s_sqwqe_to_lkey_frpmr_info_scr.rsvd3, t0_s2s_sqwqe_to_lkey_frpmr_info.rsvd3);
    modify_field(t0_s2s_sqwqe_to_lkey_frpmr_info_scr.new_user_key, t0_s2s_sqwqe_to_lkey_frpmr_info.new_user_key);
    modify_field(t0_s2s_sqwqe_to_lkey_frpmr_info_scr.acc_ctrl, t0_s2s_sqwqe_to_lkey_frpmr_info.acc_ctrl);
    modify_field(t0_s2s_sqwqe_to_lkey_frpmr_info_scr.log_page_size, t0_s2s_sqwqe_to_lkey_frpmr_info.log_page_size);
    modify_field(t0_s2s_sqwqe_to_lkey_frpmr_info_scr.base_va, t0_s2s_sqwqe_to_lkey_frpmr_info.base_va);
    modify_field(t0_s2s_sqwqe_to_lkey_frpmr_info_scr.num_pt_entries, t0_s2s_sqwqe_to_lkey_frpmr_info.num_pt_entries);
    modify_field(t0_s2s_sqwqe_to_lkey_frpmr_info_scr.rsvd2, t0_s2s_sqwqe_to_lkey_frpmr_info.rsvd2);
}

action req_tx_frpmr_sqlkey_process_t1 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_frpmr_sqlkey_info_scr.header_template_addr_or_pd, to_s4_frpmr_sqlkey_info.header_template_addr_or_pd);
    modify_field(to_s4_frpmr_sqlkey_info_scr.congestion_mgmt_type, to_s4_frpmr_sqlkey_info.congestion_mgmt_type);
    modify_field(to_s4_frpmr_sqlkey_info_scr.len, to_s4_frpmr_sqlkey_info.len);
    modify_field(to_s4_frpmr_sqlkey_info_scr.rsvd, to_s4_frpmr_sqlkey_info.rsvd);

    // stage to stage
    modify_field(t1_s2s_sqwqe_to_lkey_frpmr_info_scr.sge_index, t1_s2s_sqwqe_to_lkey_frpmr_info.sge_index);
    modify_field(t1_s2s_sqwqe_to_lkey_frpmr_info_scr.fast_reg_rsvd_lkey_enable, t1_s2s_sqwqe_to_lkey_frpmr_info.fast_reg_rsvd_lkey_enable);
    modify_field(t1_s2s_sqwqe_to_lkey_frpmr_info_scr.zbva, t1_s2s_sqwqe_to_lkey_frpmr_info.zbva);
    modify_field(t1_s2s_sqwqe_to_lkey_frpmr_info_scr.lkey_state_update, t1_s2s_sqwqe_to_lkey_frpmr_info.lkey_state_update);
    modify_field(t1_s2s_sqwqe_to_lkey_frpmr_info_scr.rsvd3, t1_s2s_sqwqe_to_lkey_frpmr_info.rsvd3);
    modify_field(t1_s2s_sqwqe_to_lkey_frpmr_info_scr.new_user_key, t1_s2s_sqwqe_to_lkey_frpmr_info.new_user_key);
    modify_field(t1_s2s_sqwqe_to_lkey_frpmr_info_scr.acc_ctrl, t1_s2s_sqwqe_to_lkey_frpmr_info.acc_ctrl);
    modify_field(t1_s2s_sqwqe_to_lkey_frpmr_info_scr.log_page_size, t1_s2s_sqwqe_to_lkey_frpmr_info.log_page_size);
    modify_field(t1_s2s_sqwqe_to_lkey_frpmr_info_scr.base_va, t1_s2s_sqwqe_to_lkey_frpmr_info.base_va);
    modify_field(t1_s2s_sqwqe_to_lkey_frpmr_info_scr.num_pt_entries, t1_s2s_sqwqe_to_lkey_frpmr_info.num_pt_entries);
    modify_field(t1_s2s_sqwqe_to_lkey_frpmr_info_scr.rsvd2, t1_s2s_sqwqe_to_lkey_frpmr_info.rsvd2);
}

action req_tx_bind_mw_sqlkey_process_s4 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_dcqcn_bind_mw_info_scr.pad, to_s4_dcqcn_bind_mw_info.pad);

    // stage to stage
    modify_field(t0_s2s_sqwqe_to_lkey_mw_info_scr.va, t0_s2s_sqwqe_to_lkey_mw_info.va);
    modify_field(t0_s2s_sqwqe_to_lkey_mw_info_scr.len, t0_s2s_sqwqe_to_lkey_mw_info.len);
    modify_field(t0_s2s_sqwqe_to_lkey_mw_info_scr.r_key, t0_s2s_sqwqe_to_lkey_mw_info.r_key);
    modify_field(t0_s2s_sqwqe_to_lkey_mw_info_scr.new_r_key_key, t0_s2s_sqwqe_to_lkey_mw_info.new_r_key_key);
    modify_field(t0_s2s_sqwqe_to_lkey_mw_info_scr.acc_ctrl, t0_s2s_sqwqe_to_lkey_mw_info.acc_ctrl);
    modify_field(t0_s2s_sqwqe_to_lkey_mw_info_scr.mw_type, t0_s2s_sqwqe_to_lkey_mw_info.mw_type);
    modify_field(t0_s2s_sqwqe_to_lkey_mw_info_scr.zbva, t0_s2s_sqwqe_to_lkey_mw_info.zbva);

}

action req_tx_sqpt_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t0_s2s_sqcb_to_pt_info_scr.page_offset, t0_s2s_sqcb_to_pt_info.page_offset);
    modify_field(t0_s2s_sqcb_to_pt_info_scr.page_seg_offset, t0_s2s_sqcb_to_pt_info.page_seg_offset);
    modify_field(t0_s2s_sqcb_to_pt_info_scr.remaining_payload_bytes, t0_s2s_sqcb_to_pt_info.remaining_payload_bytes);
    modify_field(t0_s2s_sqcb_to_pt_info_scr.rrq_p_index, t0_s2s_sqcb_to_pt_info.rrq_p_index);
    modify_field(t0_s2s_sqcb_to_pt_info_scr.pd, t0_s2s_sqcb_to_pt_info.pd);
    modify_field(t0_s2s_sqcb_to_pt_info_scr.log_pmtu, t0_s2s_sqcb_to_pt_info.log_pmtu);
    modify_field(t0_s2s_sqcb_to_pt_info_scr.poll_in_progress, t0_s2s_sqcb_to_pt_info.poll_in_progress);
    modify_field(t0_s2s_sqcb_to_pt_info_scr.color, t0_s2s_sqcb_to_pt_info.color);
    modify_field(t0_s2s_sqcb_to_pt_info_scr.read_req_adjust, t0_s2s_sqcb_to_pt_info.read_req_adjust);
    modify_field(t0_s2s_sqcb_to_pt_info_scr.spec_enable, t0_s2s_sqcb_to_pt_info.spec_enable);
    modify_field(t0_s2s_sqcb_to_pt_info_scr.pad, t0_s2s_sqcb_to_pt_info.pad);

}

action req_tx_sqptseg_process_t0 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t0_s2s_lkey_to_ptseg_info_scr.pt_offset, t0_s2s_lkey_to_ptseg_info.pt_offset);
    modify_field(t0_s2s_lkey_to_ptseg_info_scr.log_page_size, t0_s2s_lkey_to_ptseg_info.log_page_size);
    modify_field(t0_s2s_lkey_to_ptseg_info_scr.pt_bytes, t0_s2s_lkey_to_ptseg_info.pt_bytes);
    modify_field(t0_s2s_lkey_to_ptseg_info_scr.dma_cmd_start_index, t0_s2s_lkey_to_ptseg_info.dma_cmd_start_index);
    modify_field(t0_s2s_lkey_to_ptseg_info_scr.sge_index, t0_s2s_lkey_to_ptseg_info.sge_index);
    modify_field(t0_s2s_lkey_to_ptseg_info_scr.host_addr, t0_s2s_lkey_to_ptseg_info.host_addr);
    modify_field(t0_s2s_lkey_to_ptseg_info_scr.pad, t0_s2s_lkey_to_ptseg_info.pad);

}

action req_tx_bind_mw_rkey_process_s4 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_dcqcn_bind_mw_info_scr.host_addr_spec_enable, to_s4_dcqcn_bind_mw_info.host_addr_spec_enable);
    modify_field(to_s4_dcqcn_bind_mw_info_scr.mr_l_key, to_s4_dcqcn_bind_mw_info.mr_l_key);
    modify_field(to_s4_dcqcn_bind_mw_info_scr.mr_cookie_msg_psn, to_s4_dcqcn_bind_mw_info.mr_cookie_msg_psn);

    // stage to stage
    modify_field(t0_s2s_sqlkey_to_rkey_mw_info_scr.va, t0_s2s_sqlkey_to_rkey_mw_info.va);
    modify_field(t0_s2s_sqlkey_to_rkey_mw_info_scr.len, t0_s2s_sqlkey_to_rkey_mw_info.len);
    modify_field(t0_s2s_sqlkey_to_rkey_mw_info_scr.mw_pt_base, t0_s2s_sqlkey_to_rkey_mw_info.mw_pt_base);
    modify_field(t0_s2s_sqlkey_to_rkey_mw_info_scr.new_r_key_key, t0_s2s_sqlkey_to_rkey_mw_info.new_r_key_key);
    modify_field(t0_s2s_sqlkey_to_rkey_mw_info_scr.acc_ctrl, t0_s2s_sqlkey_to_rkey_mw_info.acc_ctrl);
    modify_field(t0_s2s_sqlkey_to_rkey_mw_info_scr.mw_type, t0_s2s_sqlkey_to_rkey_mw_info.mw_type);
    modify_field(t0_s2s_sqlkey_to_rkey_mw_info_scr.zbva, t0_s2s_sqlkey_to_rkey_mw_info.zbva);
    modify_field(t0_s2s_sqlkey_to_rkey_mw_info_scr.log_page_size, t0_s2s_sqlkey_to_rkey_mw_info.log_page_size);

}

action req_tx_sqptseg_process_t1 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t1_s2s_lkey_to_ptseg_info_scr.pt_offset, t1_s2s_lkey_to_ptseg_info.pt_offset);
    modify_field(t1_s2s_lkey_to_ptseg_info_scr.log_page_size, t1_s2s_lkey_to_ptseg_info.log_page_size);
    modify_field(t1_s2s_lkey_to_ptseg_info_scr.pt_bytes, t1_s2s_lkey_to_ptseg_info.pt_bytes);
    modify_field(t1_s2s_lkey_to_ptseg_info_scr.dma_cmd_start_index, t1_s2s_lkey_to_ptseg_info.dma_cmd_start_index);
    modify_field(t1_s2s_lkey_to_ptseg_info_scr.sge_index, t1_s2s_lkey_to_ptseg_info.sge_index);
    modify_field(t1_s2s_lkey_to_ptseg_info_scr.host_addr, t1_s2s_lkey_to_ptseg_info.host_addr);
    modify_field(t1_s2s_lkey_to_ptseg_info_scr.pad, t1_s2s_lkey_to_ptseg_info.pad);

}

action req_tx_add_headers_2_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s6_sqcb_wb_add_hdr_info_scr.packet_len, to_s6_sqcb_wb_add_hdr_info.packet_len);

    // stage to stage
    modify_field(t3_s2s_add_hdr_info_scr.hdr_template_inline, t3_s2s_add_hdr_info.hdr_template_inline);
    modify_field(t3_s2s_add_hdr_info_scr.service, t3_s2s_add_hdr_info.service);
    modify_field(t3_s2s_add_hdr_info_scr.header_template_addr, t3_s2s_add_hdr_info.header_template_addr);
    modify_field(t3_s2s_add_hdr_info_scr.header_template_size, t3_s2s_add_hdr_info.header_template_size);
    modify_field(t3_s2s_add_hdr_info_scr.roce_opt_ts_enable, t3_s2s_add_hdr_info.roce_opt_ts_enable);
    modify_field(t3_s2s_add_hdr_info_scr.roce_opt_mss_enable, t3_s2s_add_hdr_info.roce_opt_mss_enable);
    modify_field(t3_s2s_add_hdr_info_scr.pad, t3_s2s_add_hdr_info.pad);

}
action req_tx_credits_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
}

action req_tx_sqsge_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s3_sqsge_info_scr.header_template_addr, to_s3_sqsge_info.header_template_addr);
    modify_field(to_s3_sqsge_info_scr.packet_len, to_s3_sqsge_info.packet_len);
    modify_field(to_s3_sqsge_info_scr.congestion_mgmt_type, to_s3_sqsge_info.congestion_mgmt_type);
    modify_field(to_s3_sqsge_info_scr.priv_oper_enable, to_s3_sqsge_info.priv_oper_enable);
    modify_field(to_s3_sqsge_info_scr.pad, to_s3_sqsge_info.pad);
    modify_field(to_s3_sqsge_info_scr.spec_msg_psn, to_s3_sqsge_info.spec_msg_psn);
    modify_field(to_s3_sqsge_info_scr.spec_enable, to_s3_sqsge_info.spec_enable);


    // stage to stage
    modify_field(t0_s2s_wqe_to_sge_info_scr.in_progress, t0_s2s_wqe_to_sge_info.in_progress);
    modify_field(t0_s2s_wqe_to_sge_info_scr.op_type, t0_s2s_wqe_to_sge_info.op_type);
    modify_field(t0_s2s_wqe_to_sge_info_scr.first, t0_s2s_wqe_to_sge_info.first);
    modify_field(t0_s2s_wqe_to_sge_info_scr.current_sge_id, t0_s2s_wqe_to_sge_info.current_sge_id);
    modify_field(t0_s2s_wqe_to_sge_info_scr.num_valid_sges, t0_s2s_wqe_to_sge_info.num_valid_sges);
    modify_field(t0_s2s_wqe_to_sge_info_scr.current_sge_offset, t0_s2s_wqe_to_sge_info.current_sge_offset);
    modify_field(t0_s2s_wqe_to_sge_info_scr.remaining_payload_bytes, t0_s2s_wqe_to_sge_info.remaining_payload_bytes);
    modify_field(t0_s2s_wqe_to_sge_info_scr.poll_in_progress, t0_s2s_wqe_to_sge_info.poll_in_progress);
    modify_field(t0_s2s_wqe_to_sge_info_scr.color, t0_s2s_wqe_to_sge_info.color);
    modify_field(t0_s2s_wqe_to_sge_info_scr.rsvd, t0_s2s_wqe_to_sge_info.rsvd);
    modify_field(t0_s2s_wqe_to_sge_info_scr.dma_cmd_start_index, t0_s2s_wqe_to_sge_info.dma_cmd_start_index);
    modify_field(t0_s2s_wqe_to_sge_info_scr.imm_data_or_inv_key, t0_s2s_wqe_to_sge_info.imm_data_or_inv_key);
    modify_field(t0_s2s_wqe_to_sge_info_scr.ah_handle, t0_s2s_wqe_to_sge_info.ah_handle);
    modify_field(t0_s2s_wqe_to_sge_info_scr.spec_enable, t0_s2s_wqe_to_sge_info.spec_enable);

}

action req_tx_load_ah_size_process() {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
}

action req_tx_dummy_sqpt_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.in_progress, t0_s2s_sqcb_to_wqe_info.in_progress);
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.log_pmtu, t0_s2s_sqcb_to_wqe_info.log_pmtu);
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.li_fence_cleared, t0_s2s_sqcb_to_wqe_info.li_fence_cleared);
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.current_sge_id, t0_s2s_sqcb_to_wqe_info.current_sge_id);
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.num_valid_sges, t0_s2s_sqcb_to_wqe_info.num_valid_sges);
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.current_sge_offset, t0_s2s_sqcb_to_wqe_info.current_sge_offset);
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.remaining_payload_bytes, t0_s2s_sqcb_to_wqe_info.remaining_payload_bytes);
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.rrq_p_index, t0_s2s_sqcb_to_wqe_info.rrq_p_index);
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.pd, t0_s2s_sqcb_to_wqe_info.pd);
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.poll_in_progress, t0_s2s_sqcb_to_wqe_info.poll_in_progress);
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.color, t0_s2s_sqcb_to_wqe_info.color);
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.fence_done, t0_s2s_sqcb_to_wqe_info.fence_done);
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.pad, t0_s2s_sqcb_to_wqe_info.pad);

}

action req_tx_add_headers_process_rd () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s5_sqcb_wb_add_hdr_info_scr.wqe_addr, to_s5_sqcb_wb_add_hdr_info.wqe_addr);
    modify_field(to_s5_sqcb_wb_add_hdr_info_scr.pad, to_s5_sqcb_wb_add_hdr_info.pad);
    modify_field(to_s5_sqcb_wb_add_hdr_info_scr.ah_size, to_s5_sqcb_wb_add_hdr_info.ah_size);
    modify_field(to_s5_sqcb_wb_add_hdr_info_scr.spec_enable, to_s5_sqcb_wb_add_hdr_info.spec_enable);
    modify_field(to_s5_sqcb_wb_add_hdr_info_scr.fence, to_s5_sqcb_wb_add_hdr_info.fence);
    modify_field(to_s5_sqcb_wb_add_hdr_info_scr.read_req_adjust, to_s5_sqcb_wb_add_hdr_info.read_req_adjust);


    // stage to stage
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.hdr_template_inline, t2_s2s_sqcb_write_back_info_rd.hdr_template_inline);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.busy, t2_s2s_sqcb_write_back_info_rd.busy);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.in_progress, t2_s2s_sqcb_write_back_info_rd.in_progress);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.bktrack_in_progress, t2_s2s_sqcb_write_back_info_rd.bktrack_in_progress);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.op_type, t2_s2s_sqcb_write_back_info_rd.op_type);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.first, t2_s2s_sqcb_write_back_info_rd.first);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.last_pkt, t2_s2s_sqcb_write_back_info_rd.last_pkt);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.set_fence, t2_s2s_sqcb_write_back_info_rd.set_fence);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.set_li_fence, t2_s2s_sqcb_write_back_info_rd.set_li_fence);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.empty_rrq_bktrack, t2_s2s_sqcb_write_back_info_rd.empty_rrq_bktrack);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.current_sge_offset, t2_s2s_sqcb_write_back_info_rd.current_sge_offset);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.current_sge_id, t2_s2s_sqcb_write_back_info_rd.current_sge_id);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.num_sges, t2_s2s_sqcb_write_back_info_rd.num_sges);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.sq_c_index, t2_s2s_sqcb_write_back_info_rd.sq_c_index);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.ah_size, t2_s2s_sqcb_write_back_info_rd.ah_size);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.poll_in_progress, t2_s2s_sqcb_write_back_info_rd.poll_in_progress);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.color, t2_s2s_sqcb_write_back_info_rd.color);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.poll_failed, t2_s2s_sqcb_write_back_info_rd.poll_failed);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.rate_enforce_failed, t2_s2s_sqcb_write_back_info_rd.rate_enforce_failed);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.non_packet_wqe, t2_s2s_sqcb_write_back_info_rd.non_packet_wqe);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.rsvd, t2_s2s_sqcb_write_back_info_rd.rsvd);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.op_rd_read_len, t2_s2s_sqcb_write_back_info_rd.op_rd_read_len);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.op_rd_log_pmtu, t2_s2s_sqcb_write_back_info_rd.op_rd_log_pmtu);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.op_rd_pad, t2_s2s_sqcb_write_back_info_rd.op_rd_pad);
}

action req_tx_add_headers_process_send_wr () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s5_sqcb_wb_add_hdr_info_scr.wqe_addr, to_s5_sqcb_wb_add_hdr_info.wqe_addr);
    modify_field(to_s5_sqcb_wb_add_hdr_info_scr.pad, to_s5_sqcb_wb_add_hdr_info.pad);
    modify_field(to_s5_sqcb_wb_add_hdr_info_scr.fence, to_s5_sqcb_wb_add_hdr_info.fence);
    modify_field(to_s5_sqcb_wb_add_hdr_info_scr.spec_enable, to_s5_sqcb_wb_add_hdr_info.spec_enable);



    // stage to stage
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.hdr_template_inline,
                 t2_s2s_sqcb_write_back_info_send_wr.hdr_template_inline);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.busy,
                 t2_s2s_sqcb_write_back_info_send_wr.busy);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.in_progress,
                 t2_s2s_sqcb_write_back_info_send_wr.in_progress);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.bktrack_in_progress,
                 t2_s2s_sqcb_write_back_info_send_wr.bktrack_in_progress);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.op_type,
                 t2_s2s_sqcb_write_back_info_send_wr.op_type);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.first,
                 t2_s2s_sqcb_write_back_info_send_wr.first);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.last_pkt,
                 t2_s2s_sqcb_write_back_info_send_wr.last_pkt);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.set_fence,
                 t2_s2s_sqcb_write_back_info_send_wr.set_fence);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.set_li_fence,
                 t2_s2s_sqcb_write_back_info_send_wr.set_li_fence);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.empty_rrq_bktrack,
                 t2_s2s_sqcb_write_back_info_send_wr.empty_rrq_bktrack);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.current_sge_offset,
                 t2_s2s_sqcb_write_back_info_send_wr.current_sge_offset);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.current_sge_id,
                 t2_s2s_sqcb_write_back_info_send_wr.current_sge_id);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.num_sges,
                 t2_s2s_sqcb_write_back_info_send_wr.num_sges);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.sq_c_index,
                 t2_s2s_sqcb_write_back_info_send_wr.sq_c_index);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.ah_size,
                 t2_s2s_sqcb_write_back_info_send_wr.ah_size);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.poll_in_progress,
                 t2_s2s_sqcb_write_back_info_send_wr.poll_in_progress);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.color,
                 t2_s2s_sqcb_write_back_info_send_wr.color);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.poll_failed,
                 t2_s2s_sqcb_write_back_info_send_wr.poll_failed);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr.rate_enforce_failed,
                 t2_s2s_sqcb_write_back_info_send_wr.rate_enforce_failed);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr.non_packet_wqe,
                 t2_s2s_sqcb_write_back_info_send_wr.non_packet_wqe);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.rsvd,
                 t2_s2s_sqcb_write_back_info_send_wr.rsvd);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.op_send_wr_imm_data_or_inv_key,
                 t2_s2s_sqcb_write_back_info_send_wr.op_send_wr_imm_data_or_inv_key);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.op_send_wr_ah_handle,
                 t2_s2s_sqcb_write_back_info_send_wr.op_send_wr_ah_handle);
}


action req_tx_sqcb2_write_back_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s5_sqcb_wb_add_hdr_info_scr.wqe_addr, to_s5_sqcb_wb_add_hdr_info.wqe_addr);
    modify_field(to_s5_sqcb_wb_add_hdr_info_scr.pad, to_s5_sqcb_wb_add_hdr_info.pad);
    modify_field(to_s5_sqcb_wb_add_hdr_info_scr.fence, to_s5_sqcb_wb_add_hdr_info.fence);
    modify_field(to_s5_sqcb_wb_add_hdr_info_scr.spec_enable, to_s5_sqcb_wb_add_hdr_info.spec_enable);



    // stage to stage
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.hdr_template_inline, t2_s2s_sqcb_write_back_info_rd.hdr_template_inline);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.busy, t2_s2s_sqcb_write_back_info_rd.busy);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.in_progress, t2_s2s_sqcb_write_back_info_rd.in_progress);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.bktrack_in_progress, t2_s2s_sqcb_write_back_info_rd.bktrack_in_progress);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.op_type, t2_s2s_sqcb_write_back_info_rd.op_type);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.first, t2_s2s_sqcb_write_back_info_rd.first);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.last_pkt, t2_s2s_sqcb_write_back_info_rd.last_pkt);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.set_fence, t2_s2s_sqcb_write_back_info_rd.set_fence);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.set_li_fence, t2_s2s_sqcb_write_back_info_rd.set_li_fence);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.empty_rrq_bktrack, t2_s2s_sqcb_write_back_info_rd.empty_rrq_bktrack);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.current_sge_offset, t2_s2s_sqcb_write_back_info_rd.current_sge_offset);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.current_sge_id, t2_s2s_sqcb_write_back_info_rd.current_sge_id);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.num_sges, t2_s2s_sqcb_write_back_info_rd.num_sges);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.sq_c_index, t2_s2s_sqcb_write_back_info_rd.sq_c_index);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.ah_size, t2_s2s_sqcb_write_back_info_rd.ah_size);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.poll_in_progress, t2_s2s_sqcb_write_back_info_rd.poll_in_progress);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.color, t2_s2s_sqcb_write_back_info_rd.color);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.poll_failed, t2_s2s_sqcb_write_back_info_rd.poll_failed);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.rate_enforce_failed, t2_s2s_sqcb_write_back_info_rd.rate_enforce_failed);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.non_packet_wqe, t2_s2s_sqcb_write_back_info_rd.non_packet_wqe);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.rsvd, t2_s2s_sqcb_write_back_info_rd.rsvd);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.op_rd_read_len, t2_s2s_sqcb_write_back_info_rd.op_rd_read_len);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.op_rd_log_pmtu, t2_s2s_sqcb_write_back_info_rd.op_rd_log_pmtu);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.op_rd_pad, t2_s2s_sqcb_write_back_info_rd.op_rd_pad);
}

action req_tx_dcqcn_enforce_process_s3 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s3_sqsge_info_scr.header_template_addr, to_s3_sqsge_info.header_template_addr);
    modify_field(to_s3_sqsge_info_scr.congestion_mgmt_type, to_s3_sqsge_info.congestion_mgmt_type);
    modify_field(to_s3_sqsge_info_scr.packet_len, to_s3_sqsge_info.packet_len);

    // stage to stage
    modify_field(t2_s2s_sqcb_write_back_info_scr.hdr_template_inline, t2_s2s_sqcb_write_back_info.hdr_template_inline);
    modify_field(t2_s2s_sqcb_write_back_info_scr.busy, t2_s2s_sqcb_write_back_info.busy);
    modify_field(t2_s2s_sqcb_write_back_info_scr.in_progress, t2_s2s_sqcb_write_back_info.in_progress);
    modify_field(t2_s2s_sqcb_write_back_info_scr.bktrack_in_progress, t2_s2s_sqcb_write_back_info.bktrack_in_progress);
    modify_field(t2_s2s_sqcb_write_back_info_scr.op_type, t2_s2s_sqcb_write_back_info.op_type);
    modify_field(t2_s2s_sqcb_write_back_info_scr.first, t2_s2s_sqcb_write_back_info.first);
    modify_field(t2_s2s_sqcb_write_back_info_scr.last_pkt, t2_s2s_sqcb_write_back_info.last_pkt);
    modify_field(t2_s2s_sqcb_write_back_info_scr.set_fence, t2_s2s_sqcb_write_back_info.set_fence);
    modify_field(t2_s2s_sqcb_write_back_info_scr.set_li_fence, t2_s2s_sqcb_write_back_info.set_li_fence);
    modify_field(t2_s2s_sqcb_write_back_info_scr.empty_rrq_bktrack, t2_s2s_sqcb_write_back_info.empty_rrq_bktrack);
    modify_field(t2_s2s_sqcb_write_back_info_scr.current_sge_offset, t2_s2s_sqcb_write_back_info.current_sge_offset);
    modify_field(t2_s2s_sqcb_write_back_info_scr.current_sge_id, t2_s2s_sqcb_write_back_info.current_sge_id);
    modify_field(t2_s2s_sqcb_write_back_info_scr.num_sges, t2_s2s_sqcb_write_back_info.num_sges);
    modify_field(t2_s2s_sqcb_write_back_info_scr.sq_c_index, t2_s2s_sqcb_write_back_info.sq_c_index);
    modify_field(t2_s2s_sqcb_write_back_info_scr.ah_size, t2_s2s_sqcb_write_back_info.ah_size);
    modify_field(t2_s2s_sqcb_write_back_info_scr.poll_in_progress, t2_s2s_sqcb_write_back_info.poll_in_progress);
    modify_field(t2_s2s_sqcb_write_back_info_scr.color, t2_s2s_sqcb_write_back_info.color);
    modify_field(t2_s2s_sqcb_write_back_info_scr.poll_failed, t2_s2s_sqcb_write_back_info.poll_failed);
    modify_field(t2_s2s_sqcb_write_back_info_scr.rate_enforce_failed, t2_s2s_sqcb_write_back_info.rate_enforce_failed);
    modify_field(t2_s2s_sqcb_write_back_info_scr.non_packet_wqe, t2_s2s_sqcb_write_back_info.non_packet_wqe);
    modify_field(t2_s2s_sqcb_write_back_info_scr.rsvd, t2_s2s_sqcb_write_back_info.rsvd);

}

action req_tx_dcqcn_enforce_process_s4 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_dcqcn_bind_mw_info_scr.header_template_addr_or_pd,
                 to_s4_dcqcn_bind_mw_info.header_template_addr_or_pd);
    modify_field(to_s4_dcqcn_bind_mw_info_scr.pad,
                 to_s4_dcqcn_bind_mw_info.pad);
    modify_field(to_s4_dcqcn_bind_mw_info_scr.spec_reset,
                 to_s4_dcqcn_bind_mw_info.spec_reset);
    modify_field(to_s4_dcqcn_bind_mw_info_scr.packet_len,
                 to_s4_dcqcn_bind_mw_info.packet_len);
    modify_field(to_s4_dcqcn_bind_mw_info_scr.congestion_mgmt_type,
                 to_s4_dcqcn_bind_mw_info.congestion_mgmt_type);
    modify_field(to_s4_dcqcn_bind_mw_info_scr.mr_cookie_msg_psn,
                 to_s4_dcqcn_bind_mw_info.mr_cookie_msg_psn);
    modify_field(to_s4_dcqcn_bind_mw_info_scr.host_addr_spec_enable,
                 to_s4_dcqcn_bind_mw_info.host_addr_spec_enable);

    // stage to stage
    modify_field(t2_s2s_sqcb_write_back_info_scr.hdr_template_inline, t2_s2s_sqcb_write_back_info.hdr_template_inline);
    modify_field(t2_s2s_sqcb_write_back_info_scr.busy, t2_s2s_sqcb_write_back_info.busy);
    modify_field(t2_s2s_sqcb_write_back_info_scr.in_progress, t2_s2s_sqcb_write_back_info.in_progress);
    modify_field(t2_s2s_sqcb_write_back_info_scr.bktrack_in_progress, t2_s2s_sqcb_write_back_info.bktrack_in_progress);
    modify_field(t2_s2s_sqcb_write_back_info_scr.op_type, t2_s2s_sqcb_write_back_info.op_type);
    modify_field(t2_s2s_sqcb_write_back_info_scr.first, t2_s2s_sqcb_write_back_info.first);
    modify_field(t2_s2s_sqcb_write_back_info_scr.last_pkt, t2_s2s_sqcb_write_back_info.last_pkt);
    modify_field(t2_s2s_sqcb_write_back_info_scr.set_fence, t2_s2s_sqcb_write_back_info.set_fence);
    modify_field(t2_s2s_sqcb_write_back_info_scr.set_li_fence, t2_s2s_sqcb_write_back_info.set_li_fence);
    modify_field(t2_s2s_sqcb_write_back_info_scr.empty_rrq_bktrack, t2_s2s_sqcb_write_back_info.empty_rrq_bktrack);
    modify_field(t2_s2s_sqcb_write_back_info_scr.current_sge_offset, t2_s2s_sqcb_write_back_info.current_sge_offset);
    modify_field(t2_s2s_sqcb_write_back_info_scr.current_sge_id, t2_s2s_sqcb_write_back_info.current_sge_id);
    modify_field(t2_s2s_sqcb_write_back_info_scr.num_sges, t2_s2s_sqcb_write_back_info.num_sges);
    modify_field(t2_s2s_sqcb_write_back_info_scr.sq_c_index, t2_s2s_sqcb_write_back_info.sq_c_index);
    modify_field(t2_s2s_sqcb_write_back_info_scr.ah_size, t2_s2s_sqcb_write_back_info.ah_size);
    modify_field(t2_s2s_sqcb_write_back_info_scr.poll_in_progress, t2_s2s_sqcb_write_back_info.poll_in_progress);
    modify_field(t2_s2s_sqcb_write_back_info_scr.color, t2_s2s_sqcb_write_back_info.color);
    modify_field(t2_s2s_sqcb_write_back_info_scr.poll_failed, t2_s2s_sqcb_write_back_info.poll_failed);
    modify_field(t2_s2s_sqcb_write_back_info_scr.rate_enforce_failed, t2_s2s_sqcb_write_back_info.rate_enforce_failed);
    modify_field(t2_s2s_sqcb_write_back_info_scr.non_packet_wqe, t2_s2s_sqcb_write_back_info.non_packet_wqe);
    modify_field(t2_s2s_sqcb_write_back_info_scr.rsvd, t2_s2s_sqcb_write_back_info.rsvd);

}

action req_tx_load_hdr_template_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage

}

action req_tx_sqcb_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage

}
action req_tx_sqwqe_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s2_sqwqe_info_scr.wqe_addr, to_s2_sqwqe_info.wqe_addr);
    modify_field(to_s2_sqwqe_info_scr.header_template_addr,
                 to_s2_sqwqe_info.header_template_addr);
    modify_field(to_s2_sqwqe_info_scr.fast_reg_rsvd_lkey_enable, to_s2_sqwqe_info.fast_reg_rsvd_lkey_enable);
    modify_field(to_s2_sqwqe_info_scr.pad, to_s2_sqwqe_info.pad);
    modify_field(to_s2_sqwqe_info_scr.log_num_kt_entries, to_s2_sqwqe_info.log_num_kt_entries);
    modify_field(to_s2_sqwqe_info_scr.log_num_dcqcn_profiles, to_s2_sqwqe_info.log_num_dcqcn_profiles);

    // stage to stage
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.in_progress, t0_s2s_sqcb_to_wqe_info.in_progress);
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.spec_enable, t0_s2s_sqcb_to_wqe_info.spec_enable);
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.log_pmtu, t0_s2s_sqcb_to_wqe_info.log_pmtu);
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.li_fence_cleared, t0_s2s_sqcb_to_wqe_info.li_fence_cleared);
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.current_sge_id, t0_s2s_sqcb_to_wqe_info.current_sge_id);
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.num_valid_sges, t0_s2s_sqcb_to_wqe_info.num_valid_sges);
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.current_sge_offset, t0_s2s_sqcb_to_wqe_info.current_sge_offset);
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.remaining_payload_bytes, t0_s2s_sqcb_to_wqe_info.remaining_payload_bytes);
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.rrq_p_index, t0_s2s_sqcb_to_wqe_info.rrq_p_index);
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.pd, t0_s2s_sqcb_to_wqe_info.pd);
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.poll_in_progress, t0_s2s_sqcb_to_wqe_info.poll_in_progress);
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.color, t0_s2s_sqcb_to_wqe_info.color);
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.fence_done, t0_s2s_sqcb_to_wqe_info.fence_done);
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.frpmr_lkey_state_upd, t0_s2s_sqcb_to_wqe_info.frpmr_lkey_state_upd);
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.dma_cmd_start_index, t0_s2s_sqcb_to_wqe_info.dma_cmd_start_index);
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.pad, t0_s2s_sqcb_to_wqe_info.pad);
}

action req_tx_write_back_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s6_sqcb_wb_add_hdr_info_scr.wqe_addr, to_s6_sqcb_wb_add_hdr_info.wqe_addr);
    modify_field(to_s6_sqcb_wb_add_hdr_info_scr.pad, to_s6_sqcb_wb_add_hdr_info.pad);
    modify_field(to_s6_sqcb_wb_add_hdr_info_scr.fence, to_s6_sqcb_wb_add_hdr_info.fence);
    modify_field(to_s6_sqcb_wb_add_hdr_info_scr.read_req_adjust, to_s6_sqcb_wb_add_hdr_info.read_req_adjust);



    // stage to stage
    modify_field(t2_s2s_sqcb_write_back_info_scr.hdr_template_inline, t2_s2s_sqcb_write_back_info.hdr_template_inline);
    modify_field(t2_s2s_sqcb_write_back_info_scr.busy, t2_s2s_sqcb_write_back_info.busy);
    modify_field(t2_s2s_sqcb_write_back_info_scr.in_progress, t2_s2s_sqcb_write_back_info.in_progress);
    modify_field(t2_s2s_sqcb_write_back_info_scr.bktrack_in_progress, t2_s2s_sqcb_write_back_info.bktrack_in_progress);
    modify_field(t2_s2s_sqcb_write_back_info_scr.op_type, t2_s2s_sqcb_write_back_info.op_type);
    modify_field(t2_s2s_sqcb_write_back_info_scr.first, t2_s2s_sqcb_write_back_info.first);
    modify_field(t2_s2s_sqcb_write_back_info_scr.last_pkt, t2_s2s_sqcb_write_back_info.last_pkt);
    modify_field(t2_s2s_sqcb_write_back_info_scr.set_fence, t2_s2s_sqcb_write_back_info.set_fence);
    modify_field(t2_s2s_sqcb_write_back_info_scr.set_li_fence, t2_s2s_sqcb_write_back_info.set_li_fence);
    modify_field(t2_s2s_sqcb_write_back_info_scr.empty_rrq_bktrack, t2_s2s_sqcb_write_back_info.empty_rrq_bktrack);
    modify_field(t2_s2s_sqcb_write_back_info_scr.current_sge_offset, t2_s2s_sqcb_write_back_info.current_sge_offset);
    modify_field(t2_s2s_sqcb_write_back_info_scr.current_sge_id, t2_s2s_sqcb_write_back_info.current_sge_id);
    modify_field(t2_s2s_sqcb_write_back_info_scr.num_sges, t2_s2s_sqcb_write_back_info.num_sges);
    modify_field(t2_s2s_sqcb_write_back_info_scr.sq_c_index, t2_s2s_sqcb_write_back_info.sq_c_index);
    modify_field(t2_s2s_sqcb_write_back_info_scr.ah_size, t2_s2s_sqcb_write_back_info.ah_size);
    modify_field(t2_s2s_sqcb_write_back_info_scr.poll_in_progress, t2_s2s_sqcb_write_back_info.poll_in_progress);
    modify_field(t2_s2s_sqcb_write_back_info_scr.color, t2_s2s_sqcb_write_back_info.color);
    modify_field(t2_s2s_sqcb_write_back_info_scr.poll_failed, t2_s2s_sqcb_write_back_info.poll_failed);
    modify_field(t2_s2s_sqcb_write_back_info_scr.rate_enforce_failed, t2_s2s_sqcb_write_back_info.rate_enforce_failed);
    modify_field(t2_s2s_sqcb_write_back_info_scr.non_packet_wqe, t2_s2s_sqcb_write_back_info.non_packet_wqe);
    modify_field(t2_s2s_sqcb_write_back_info_scr.credit_check_failed, t2_s2s_sqcb_write_back_info.credit_check_failed);
    modify_field(t2_s2s_sqcb_write_back_info_scr.drop_phv, t2_s2s_sqcb_write_back_info.drop_phv);
    modify_field(t2_s2s_sqcb_write_back_info_scr.rsvd, t2_s2s_sqcb_write_back_info.rsvd);

}


action req_tx_frpmr_write_back_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s6_frpmr_sqcb_wb_info_scr.frpmr_dma_src_addr, to_s6_frpmr_sqcb_wb_info.frpmr_dma_src_addr);
    modify_field(to_s6_frpmr_sqcb_wb_info_scr.wqe_addr, to_s6_frpmr_sqcb_wb_info.wqe_addr);

    // stage to stage
    modify_field(t2_s2s_frpmr_write_back_info_scr.pt_base, t2_s2s_frpmr_write_back_info.pt_base);
    modify_field(t2_s2s_frpmr_write_back_info_scr.dma_size, t2_s2s_frpmr_write_back_info.dma_size);
    modify_field(t2_s2s_frpmr_write_back_info_scr.rsvd2, t2_s2s_frpmr_write_back_info.rsvd2);
}

action req_tx_write_back_process_rd () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s6_sqcb_wb_add_hdr_info_scr.wqe_addr, to_s6_sqcb_wb_add_hdr_info.wqe_addr);
    modify_field(to_s6_sqcb_wb_add_hdr_info_scr.pad, to_s6_sqcb_wb_add_hdr_info.pad);
    modify_field(to_s6_sqcb_wb_add_hdr_info_scr.fence, to_s6_sqcb_wb_add_hdr_info.fence);


    // stage to stage
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.hdr_template_inline, t2_s2s_sqcb_write_back_info_rd.hdr_template_inline);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.busy, t2_s2s_sqcb_write_back_info_rd.busy);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.in_progress, t2_s2s_sqcb_write_back_info_rd.in_progress);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.bktrack_in_progress, t2_s2s_sqcb_write_back_info_rd.bktrack_in_progress);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.op_type, t2_s2s_sqcb_write_back_info_rd.op_type);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.first, t2_s2s_sqcb_write_back_info_rd.first);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.last_pkt, t2_s2s_sqcb_write_back_info_rd.last_pkt);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.set_fence, t2_s2s_sqcb_write_back_info_rd.set_fence);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.set_li_fence, t2_s2s_sqcb_write_back_info_rd.set_li_fence);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.empty_rrq_bktrack, t2_s2s_sqcb_write_back_info_rd.empty_rrq_bktrack);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.current_sge_offset, t2_s2s_sqcb_write_back_info_rd.current_sge_offset);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.current_sge_id, t2_s2s_sqcb_write_back_info_rd.current_sge_id);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.num_sges, t2_s2s_sqcb_write_back_info_rd.num_sges);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.sq_c_index, t2_s2s_sqcb_write_back_info_rd.sq_c_index);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.ah_size, t2_s2s_sqcb_write_back_info_rd.ah_size);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.poll_in_progress, t2_s2s_sqcb_write_back_info_rd.poll_in_progress);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.color, t2_s2s_sqcb_write_back_info_rd.color);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.poll_failed, t2_s2s_sqcb_write_back_info_rd.poll_failed);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.rate_enforce_failed, t2_s2s_sqcb_write_back_info_rd.rate_enforce_failed);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.non_packet_wqe, t2_s2s_sqcb_write_back_info_rd.non_packet_wqe);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.credit_check_failed, t2_s2s_sqcb_write_back_info_rd.credit_check_failed);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.drop_phv, t2_s2s_sqcb_write_back_info_rd.drop_phv);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.rsvd, t2_s2s_sqcb_write_back_info_rd.rsvd);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.op_rd_read_len, t2_s2s_sqcb_write_back_info_rd.op_rd_read_len);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.op_rd_log_pmtu, t2_s2s_sqcb_write_back_info_rd.op_rd_log_pmtu);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.op_rd_pad, t2_s2s_sqcb_write_back_info_rd.op_rd_pad);
}

action req_tx_write_back_process_send_wr () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s6_sqcb_wb_add_hdr_info_scr.wqe_addr, to_s6_sqcb_wb_add_hdr_info.wqe_addr);
    modify_field(to_s6_sqcb_wb_add_hdr_info_scr.pad, to_s6_sqcb_wb_add_hdr_info.pad);
    modify_field(to_s6_sqcb_wb_add_hdr_info_scr.fence, to_s6_sqcb_wb_add_hdr_info.fence);


    // stage to stage
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.hdr_template_inline,
                 t2_s2s_sqcb_write_back_info_send_wr.hdr_template_inline);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.busy,
                 t2_s2s_sqcb_write_back_info_send_wr.busy);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.in_progress,
                 t2_s2s_sqcb_write_back_info_send_wr.in_progress);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.bktrack_in_progress,
                 t2_s2s_sqcb_write_back_info_send_wr.bktrack_in_progress);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.op_type,
                 t2_s2s_sqcb_write_back_info_send_wr.op_type);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.first,
                 t2_s2s_sqcb_write_back_info_send_wr.first);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.last_pkt,
                 t2_s2s_sqcb_write_back_info_send_wr.last_pkt);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.set_fence,
                 t2_s2s_sqcb_write_back_info_send_wr.set_fence);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.set_li_fence,
                 t2_s2s_sqcb_write_back_info_send_wr.set_li_fence);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.empty_rrq_bktrack,
                 t2_s2s_sqcb_write_back_info_send_wr.empty_rrq_bktrack);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.current_sge_offset,
                 t2_s2s_sqcb_write_back_info_send_wr.current_sge_offset);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.current_sge_id,
                 t2_s2s_sqcb_write_back_info_send_wr.current_sge_id);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.num_sges,
                 t2_s2s_sqcb_write_back_info_send_wr.num_sges);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.sq_c_index,
                 t2_s2s_sqcb_write_back_info_send_wr.sq_c_index);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.ah_size,
                 t2_s2s_sqcb_write_back_info_send_wr.ah_size);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.poll_in_progress,
                 t2_s2s_sqcb_write_back_info_send_wr.poll_in_progress);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.color,
                 t2_s2s_sqcb_write_back_info_send_wr.color);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.poll_failed,
                 t2_s2s_sqcb_write_back_info_send_wr.poll_failed);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.rate_enforce_failed,
                 t2_s2s_sqcb_write_back_info_send_wr.rate_enforce_failed);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.non_packet_wqe,
                 t2_s2s_sqcb_write_back_info_send_wr.non_packet_wqe);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.credit_check_failed,
                 t2_s2s_sqcb_write_back_info_send_wr.credit_check_failed);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.drop_phv,
                 t2_s2s_sqcb_write_back_info_send_wr.drop_phv);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.rsvd,
                 t2_s2s_sqcb_write_back_info_send_wr.rsvd);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.op_send_wr_imm_data_or_inv_key,
                 t2_s2s_sqcb_write_back_info_send_wr.op_send_wr_imm_data_or_inv_key);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.op_send_wr_ah_handle,
                 t2_s2s_sqcb_write_back_info_send_wr.op_send_wr_ah_handle);
}

action req_tx_sqsge_iterate_process_s1 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t2_s2s_wqe_to_sge_info_scr.in_progress, t2_s2s_wqe_to_sge_info.in_progress);
    modify_field(t2_s2s_wqe_to_sge_info_scr.op_type, t2_s2s_wqe_to_sge_info.op_type);
    modify_field(t2_s2s_wqe_to_sge_info_scr.first, t2_s2s_wqe_to_sge_info.first);
    modify_field(t2_s2s_wqe_to_sge_info_scr.current_sge_id, t2_s2s_wqe_to_sge_info.current_sge_id);
    modify_field(t2_s2s_wqe_to_sge_info_scr.num_valid_sges, t2_s2s_wqe_to_sge_info.num_valid_sges);
    modify_field(t2_s2s_wqe_to_sge_info_scr.current_sge_offset, t2_s2s_wqe_to_sge_info.current_sge_offset);
    modify_field(t2_s2s_wqe_to_sge_info_scr.remaining_payload_bytes, t2_s2s_wqe_to_sge_info.remaining_payload_bytes);
    modify_field(t2_s2s_wqe_to_sge_info_scr.poll_in_progress, t2_s2s_wqe_to_sge_info.poll_in_progress);
    modify_field(t2_s2s_wqe_to_sge_info_scr.color, t2_s2s_wqe_to_sge_info.color);
    modify_field(t2_s2s_wqe_to_sge_info_scr.rsvd, t2_s2s_wqe_to_sge_info.rsvd);
    modify_field(t2_s2s_wqe_to_sge_info_scr.dma_cmd_start_index, t2_s2s_wqe_to_sge_info.dma_cmd_start_index);
    modify_field(t2_s2s_wqe_to_sge_info_scr.imm_data_or_inv_key, t2_s2s_wqe_to_sge_info.imm_data_or_inv_key);
    modify_field(t2_s2s_wqe_to_sge_info_scr.ah_handle, t2_s2s_wqe_to_sge_info.ah_handle);
    modify_field(t2_s2s_wqe_to_sge_info_scr.spec_enable, t2_s2s_wqe_to_sge_info.spec_enable);
}

action req_tx_sqsge_iterate_process_s2 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s2_sqwqe_info_scr.wqe_addr, to_s2_sqwqe_info.wqe_addr);

    // stage to stage
    modify_field(t2_s2s_wqe_to_sge_info_scr.in_progress, t2_s2s_wqe_to_sge_info.in_progress);
    modify_field(t2_s2s_wqe_to_sge_info_scr.op_type, t2_s2s_wqe_to_sge_info.op_type);
    modify_field(t2_s2s_wqe_to_sge_info_scr.first, t2_s2s_wqe_to_sge_info.first);
    modify_field(t2_s2s_wqe_to_sge_info_scr.current_sge_id, t2_s2s_wqe_to_sge_info.current_sge_id);
    modify_field(t2_s2s_wqe_to_sge_info_scr.num_valid_sges, t2_s2s_wqe_to_sge_info.num_valid_sges);
    modify_field(t2_s2s_wqe_to_sge_info_scr.current_sge_offset, t2_s2s_wqe_to_sge_info.current_sge_offset);
    modify_field(t2_s2s_wqe_to_sge_info_scr.remaining_payload_bytes, t2_s2s_wqe_to_sge_info.remaining_payload_bytes);
    modify_field(t2_s2s_wqe_to_sge_info_scr.poll_in_progress, t2_s2s_wqe_to_sge_info.poll_in_progress);
    modify_field(t2_s2s_wqe_to_sge_info_scr.color, t2_s2s_wqe_to_sge_info.color);
    modify_field(t2_s2s_wqe_to_sge_info_scr.rsvd, t2_s2s_wqe_to_sge_info.rsvd);
    modify_field(t2_s2s_wqe_to_sge_info_scr.dma_cmd_start_index, t2_s2s_wqe_to_sge_info.dma_cmd_start_index);
    modify_field(t2_s2s_wqe_to_sge_info_scr.imm_data_or_inv_key, t2_s2s_wqe_to_sge_info.imm_data_or_inv_key);
    modify_field(t2_s2s_wqe_to_sge_info_scr.ah_handle, t2_s2s_wqe_to_sge_info.ah_handle);
    modify_field(t2_s2s_wqe_to_sge_info_scr.spec_enable, t2_s2s_wqe_to_sge_info.spec_enable);
}

action req_tx_sqcb2_fence_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_fence_info_scr.wqe_addr, to_s1_fence_info.wqe_addr);
    modify_field(to_s1_fence_info_scr.bktrack_fence_marker_phv, to_s1_fence_info.bktrack_fence_marker_phv);
    modify_field(to_s1_fence_info_scr.pad, to_s1_fence_info.pad);

    // stage to stage
}

action req_tx_stats_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s7_stats_info_scr.pyld_bytes, to_s7_stats_info.pyld_bytes);
    modify_field(to_s7_stats_info_scr.npg, to_s7_stats_info.npg);
    modify_field(to_s7_stats_info_scr.npg_bindmw_t1, to_s7_stats_info.npg_bindmw_t1);
    modify_field(to_s7_stats_info_scr.npg_bindmw_t2, to_s7_stats_info.npg_bindmw_t2);
    modify_field(to_s7_stats_info_scr.npg_frpmr, to_s7_stats_info.npg_frpmr);
    modify_field(to_s7_stats_info_scr.npg_local_inv, to_s7_stats_info.npg_local_inv);
    modify_field(to_s7_stats_info_scr.timeout, to_s7_stats_info.timeout);
    modify_field(to_s7_stats_info_scr.timeout_local_ack, to_s7_stats_info.timeout_local_ack);
    modify_field(to_s7_stats_info_scr.timeout_rnr, to_s7_stats_info.timeout_rnr);
    modify_field(to_s7_stats_info_scr.sq_drain, to_s7_stats_info.sq_drain);
    modify_field(to_s7_stats_info_scr.lif_cqe_error_id_vld, to_s7_stats_info.lif_cqe_error_id_vld);
    modify_field(to_s7_stats_info_scr.lif_error_id_vld, to_s7_stats_info.lif_error_id_vld);
    modify_field(to_s7_stats_info_scr.lif_error_id, to_s7_stats_info.lif_error_id);
    modify_field(to_s7_stats_info_scr.qp_err_disabled, to_s7_stats_info.qp_err_disabled);
    modify_field(to_s7_stats_info_scr.qp_err_dis_flush_rq, to_s7_stats_info.qp_err_dis_flush_rq);
    modify_field(to_s7_stats_info_scr.qp_err_dis_ud_pmtu, to_s7_stats_info.qp_err_dis_ud_pmtu);
    modify_field(to_s7_stats_info_scr.qp_err_dis_ud_fast_reg, to_s7_stats_info.qp_err_dis_ud_fast_reg);
    modify_field(to_s7_stats_info_scr.qp_err_dis_ud_priv, to_s7_stats_info.qp_err_dis_ud_priv);
    modify_field(to_s7_stats_info_scr.qp_err_dis_no_dma_cmds, to_s7_stats_info.qp_err_dis_no_dma_cmds);
    modify_field(to_s7_stats_info_scr.qp_err_dis_lkey_inv_state, to_s7_stats_info.qp_err_dis_lkey_inv_state);
    modify_field(to_s7_stats_info_scr.qp_err_dis_lkey_inv_pd, to_s7_stats_info.qp_err_dis_lkey_inv_pd);
    modify_field(to_s7_stats_info_scr.qp_err_dis_lkey_rsvd_lkey, to_s7_stats_info.qp_err_dis_lkey_rsvd_lkey);
    modify_field(to_s7_stats_info_scr.qp_err_dis_lkey_access_violation, to_s7_stats_info.qp_err_dis_lkey_access_violation);
    modify_field(to_s7_stats_info_scr.qp_err_dis_bind_mw_len_exceeded, to_s7_stats_info.qp_err_dis_bind_mw_len_exceeded);
    modify_field(to_s7_stats_info_scr.qp_err_dis_bind_mw_rkey_inv_pd, to_s7_stats_info.qp_err_dis_bind_mw_rkey_inv_pd);
    modify_field(to_s7_stats_info_scr.qp_err_dis_bind_mw_rkey_inv_zbva, to_s7_stats_info.qp_err_dis_bind_mw_rkey_inv_zbva);
    modify_field(to_s7_stats_info_scr.qp_err_dis_bind_mw_rkey_inv_len, to_s7_stats_info.qp_err_dis_bind_mw_rkey_inv_len);
    modify_field(to_s7_stats_info_scr.qp_err_dis_bind_mw_rkey_inv_mw_state, to_s7_stats_info.qp_err_dis_bind_mw_rkey_inv_mw_state);
    modify_field(to_s7_stats_info_scr.qp_err_dis_bind_mw_rkey_type_disallowed, to_s7_stats_info.qp_err_dis_bind_mw_rkey_type_disallowed);
    modify_field(to_s7_stats_info_scr.qp_err_dis_bind_mw_lkey_state_valid, to_s7_stats_info.qp_err_dis_bind_mw_lkey_state_valid);
    modify_field(to_s7_stats_info_scr.qp_err_dis_bind_mw_lkey_no_bind, to_s7_stats_info.qp_err_dis_bind_mw_lkey_no_bind);
    modify_field(to_s7_stats_info_scr.qp_err_dis_bind_mw_lkey_zero_based, to_s7_stats_info.qp_err_dis_bind_mw_lkey_zero_based);
    modify_field(to_s7_stats_info_scr.qp_err_dis_bind_mw_lkey_invalid_acc_ctrl, to_s7_stats_info.qp_err_dis_bind_mw_lkey_invalid_acc_ctrl);
    modify_field(to_s7_stats_info_scr.qp_err_dis_bind_mw_lkey_invalid_va, to_s7_stats_info.qp_err_dis_bind_mw_lkey_invalid_va);
    modify_field(to_s7_stats_info_scr.qp_err_dis_bktrack_inv_num_sges, to_s7_stats_info.qp_err_dis_bktrack_inv_num_sges);
    modify_field(to_s7_stats_info_scr.qp_err_dis_bktrack_inv_rexmit_psn, to_s7_stats_info.qp_err_dis_bktrack_inv_rexmit_psn);
    modify_field(to_s7_stats_info_scr.qp_err_dis_frpmr_fast_reg_not_enabled, to_s7_stats_info.qp_err_dis_frpmr_fast_reg_not_enabled);
    modify_field(to_s7_stats_info_scr.qp_err_dis_frpmr_invalid_pd, to_s7_stats_info.qp_err_dis_frpmr_invalid_pd);
    modify_field(to_s7_stats_info_scr.qp_err_dis_frpmr_invalid_state, to_s7_stats_info.qp_err_dis_frpmr_invalid_state);
    modify_field(to_s7_stats_info_scr.qp_err_dis_frpmr_invalid_len, to_s7_stats_info.qp_err_dis_frpmr_invalid_len);
    modify_field(to_s7_stats_info_scr.qp_err_dis_frpmr_ukey_not_enabled, to_s7_stats_info.qp_err_dis_frpmr_ukey_not_enabled);
    modify_field(to_s7_stats_info_scr.qp_err_dis_inv_lkey_qp_mismatch, to_s7_stats_info.qp_err_dis_inv_lkey_qp_mismatch);
    modify_field(to_s7_stats_info_scr.qp_err_dis_inv_lkey_pd_mismatch, to_s7_stats_info.qp_err_dis_inv_lkey_pd_mismatch);
    modify_field(to_s7_stats_info_scr.qp_err_dis_inv_lkey_invalid_state, to_s7_stats_info.qp_err_dis_inv_lkey_invalid_state);
    modify_field(to_s7_stats_info_scr.qp_err_dis_inv_lkey_inv_not_allowed, to_s7_stats_info.qp_err_dis_inv_lkey_inv_not_allowed);
    modify_field(to_s7_stats_info_scr.qp_err_dis_table_error, to_s7_stats_info.qp_err_dis_table_error);
    modify_field(to_s7_stats_info_scr.qp_err_dis_phv_intrinsic_error, to_s7_stats_info.qp_err_dis_phv_intrinsic_error);
    modify_field(to_s7_stats_info_scr.qp_err_dis_table_resp_error, to_s7_stats_info.qp_err_dis_table_resp_error);
    modify_field(to_s7_stats_info_scr.qp_err_dis_inv_optype, to_s7_stats_info.qp_err_dis_inv_optype);
    modify_field(to_s7_stats_info_scr.qp_err_dis_err_retry_exceed, to_s7_stats_info.qp_err_dis_err_retry_exceed);
    modify_field(to_s7_stats_info_scr.qp_err_dis_rnr_retry_exceed, to_s7_stats_info.qp_err_dis_rnr_retry_exceed);
    modify_field(to_s7_stats_info_scr.qp_err_dis_rsvd, to_s7_stats_info.qp_err_dis_rsvd);
    modify_field(to_s7_stats_info_scr.dcqcn_cnp_sent, to_s7_stats_info.dcqcn_cnp_sent);
    modify_field(to_s7_stats_info_scr.np_cnp_sent, to_s7_stats_info.np_cnp_sent);
    modify_field(to_s7_stats_info_scr.rp_num_byte_threshold_db, to_s7_stats_info.rp_num_byte_threshold_db);
    modify_field(to_s7_stats_info_scr.pad, to_s7_stats_info.pad);

    // stage to stage
    modify_field(t3_s2s_stats_info_scr.pad, t3_s2s_stats_info.pad);

}
