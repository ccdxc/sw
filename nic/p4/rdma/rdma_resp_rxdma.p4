/***********************************************************************/
/* rdma_resp_rxdma.p4 */
/***********************************************************************/

#include "../common-p4+/common_rxdma_dummy.p4"

//#define common_p4plus_stage0_app_header_table rdma_stage0_table
#define common_p4plus_stage0_app_header_table_action_dummy rdma_stage0_bth_action
#define common_p4plus_stage0_app_header_table_action_dummy1 rdma_stage0_bth_immeth_action
#define common_p4plus_stage0_app_header_table_action_dummy2 rdma_stage0_bth_reth_action
#define common_p4plus_stage0_app_header_table_action_dummy3 rdma_stage0_bth_reth_immeth_action
#define common_p4plus_stage0_app_header_table_action_dummy4 rdma_stage0_bth_aeth_action
#define common_p4plus_stage0_app_header_table_action_dummy5 rdma_stage0_bth_aeth_atomicaeth_action
#define common_p4plus_stage0_app_header_table_action_dummy6 rdma_stage0_bth_atomiceth_action
#define common_p4plus_stage0_app_header_table_action_dummy7 rdma_stage0_bth_ieth_action
#define common_p4plus_stage0_app_header_table_action_dummy8 rdma_stage0_bth_deth_action
#define common_p4plus_stage0_app_header_table_action_dummy9 rdma_stage0_bth_deth_immeth_action
#define common_p4plus_stage0_app_header_table_action_dummy10 rdma_stage0_bth_xrceth_action
#define common_p4plus_stage0_app_header_table_action_dummy11 rdma_stage0_bth_xrceth_immeth_action
#define common_p4plus_stage0_app_header_table_action_dummy12 rdma_stage0_bth_xrceth_reth_action
#define common_p4plus_stage0_app_header_table_action_dummy13 rdma_stage0_bth_xrceth_reth_immeth_action
#define common_p4plus_stage0_app_header_table_action_dummy14 rdma_stage0_bth_xrceth_atomiceth_action
#define common_p4plus_stage0_app_header_table_action_dummy15 rdma_stage0_bth_xrceth_ieth_action
#define common_p4plus_stage0_app_header_table_action_dummy16 rdma_stage0_recirc_action
#define common_p4plus_stage0_app_header_table_action_dummy17 rdma_stage0_completion_feedback_action

#define rx_stage0_load_rdma_params_dummy1 rdma_stage0_ext_bth_atomiceth_action
#define rx_stage0_load_rdma_params_dummy2 rdma_stage0_ext_bth_xrceth_atomiceth_action
#define rx_stage0_load_rdma_params_dummy3 rdma_stage0_ext_bth_xrceth_reth_immeth_action 
#define rx_stage0_load_rdma_params_dummy4 rdma_stage0_ext_bth_deth_immeth_action
#define rx_stage0_load_rdma_params_dummy5 rdma_stage0_ext_bth_deth_action

/**** table declarations ****/

#define rx_table_s0_t0 resp_rx_s0_t0
#define rx_table_s0_t1 resp_rx_s0_t1
#define rx_table_s0_t2 resp_rx_s0_t2
#define rx_table_s0_t3 resp_rx_s0_t3

#define rx_table_s1_t0 resp_rx_s1_t0
#define rx_table_s1_t1 resp_rx_s1_t1
#define rx_table_s1_t2 resp_rx_s1_t2
#define rx_table_s1_t3 resp_rx_s1_t3

#define rx_table_s2_t0 resp_rx_s2_t0
#define rx_table_s2_t1 resp_rx_s2_t1
#define rx_table_s2_t2 resp_rx_s2_t2
#define rx_table_s2_t3 resp_rx_s2_t3

#define rx_table_s3_t0 resp_rx_s3_t0
#define rx_table_s3_t1 resp_rx_s3_t1
#define rx_table_s3_t2 resp_rx_s3_t2
#define rx_table_s3_t3 resp_rx_s3_t3

#define rx_table_s4_t0 resp_rx_s4_t0
#define rx_table_s4_t1 resp_rx_s4_t1
#define rx_table_s4_t2 resp_rx_s4_t2
#define rx_table_s4_t3 resp_rx_s4_t3

#define rx_table_s5_t0 resp_rx_s5_t0
#define rx_table_s5_t1 resp_rx_s5_t1
#define rx_table_s5_t2 resp_rx_s5_t2
#define rx_table_s5_t3 resp_rx_s5_t3

#define rx_table_s6_t0 resp_rx_s6_t0
#define rx_table_s6_t1 resp_rx_s6_t1
#define rx_table_s6_t2 resp_rx_s6_t2
#define rx_table_s6_t3 resp_rx_s6_t3

#define rx_table_s7_t0 resp_rx_s7_t0
#define rx_table_s7_t1 resp_rx_s7_t1
#define rx_table_s7_t2 resp_rx_s7_t2
#define rx_table_s7_t3 resp_rx_s7_t3


#define rx_table_s0_t0_action resp_rx_rqcb_process

#define rx_table_s1_t0_action  resp_rx_rqpt_process
#define rx_table_s1_t0_action1 resp_rx_rsq_backtrack_process
#define rx_table_s1_t0_action2 resp_rx_rqcb1_recirc_sge_process
#define rx_table_s1_t0_action3 resp_rx_rqcb3_in_progress_process
#define rx_table_s1_t0_action4 resp_rx_dummy_rqpt_process

#define rx_table_s1_t1_action resp_rx_write_dummy_process
#define rx_table_s1_t1_action1 resp_rx_read_mpu_only_process
#define rx_table_s1_t1_action2 resp_rx_atomic_resource_process
#define rx_table_s1_t2_action resp_rx_dcqcn_cnp_process
#define rx_table_s1_t2_action1 resp_rx_recirc_mpu_only_process
#define rx_table_s1_t2_action2 resp_rx_rome_cnp_process

#define rx_table_s1_t3_action resp_rx_dcqcn_ecn_process
#define rx_table_s1_t3_action1 resp_rx_rome_pkt_process

#define rx_table_s2_t0_action resp_rx_rqwqe_process
#define rx_table_s2_t0_action1 resp_rx_rsq_backtrack_adjust_process
#define rx_table_s2_t0_action2 resp_rx_rqwqe_opt_process
#define rx_table_s2_t1_action  resp_rx_rqrkey_process

#define rx_table_s3_t0_action1 resp_rx_rqwqe_wrid_process
#define rx_table_s3_t0_action2 resp_rx_rqsge_process

#define rx_table_s4_t0_action resp_rx_rqlkey_mr_cookie_process

#define rx_table_s4_t1_action resp_rx_rqlkey_process
#define rx_table_s4_t2_action resp_rx_rqcb1_write_back_mpu_only_process

#define rx_table_s4_t3_action resp_rx_inv_rkey_validate_process

#define rx_table_s5_t0_action resp_rx_ptseg_process

#define rx_table_s5_t2_action resp_rx_rqcb1_write_back_process
#define rx_table_s5_t2_action1 resp_rx_rqcb1_write_back_err_process

#define rx_table_s6_t0_action resp_rx_inv_rkey_process

#define rx_table_s6_t2_action resp_rx_cqcb_process

#define rx_table_s7_t2_action resp_rx_cqpt_process

#define rx_table_s7_t0_action resp_rx_eqcb_process_t0
#define rx_table_s7_t0_action1 resp_rx_phv_drop_mpu_only_process

#define rx_table_s7_t1_action resp_rx_eqcb_process_t1

#define rx_table_s7_t3_action resp_rx_stats_process

#include "../common-p4+/common_rxdma.p4"

#include "./rdma_rxdma_headers.p4"

/**** Macros ****/

#define GENERATE_GLOBAL_K \
    modify_field(phv_global_common_scr.lif, phv_global_common.lif);\
    modify_field(phv_global_common_scr.qid, phv_global_common.qid);\
    modify_field(phv_global_common_scr.qtype, phv_global_common.qtype);\
    modify_field(phv_global_common_scr.cb_addr, phv_global_common.cb_addr);\
    modify_field(phv_global_common_scr.pt_base_addr_page_id, phv_global_common.pt_base_addr_page_id);\
    modify_field(phv_global_common_scr.log_num_pt_entries, phv_global_common.log_num_pt_entries);\
    modify_field(phv_global_common_scr.pad, phv_global_common.pad);\
    modify_field(phv_global_common_scr._ud, phv_global_common._ud);\
    modify_field(phv_global_common_scr._ring_dbell, phv_global_common._ring_dbell);\
    modify_field(phv_global_common_scr._ack_req, phv_global_common._ack_req);\
    modify_field(phv_global_common_scr._completion, phv_global_common._completion);\
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

header_type resp_rx_to_stage_ext_hdr_info_t {
    fields {
        ext_hdr_data                     :   69;
        rsvd                             :   27;
        pd                               :   32;
    }
}

header_type resp_rx_key_info_t {
    fields {
        va                               :   64;
        current_sge_offset               :   32;
        len                              :   32;
        dma_cmd_start_index              :    8;
        tbl_id                           :    3;
        dma_cmdeop                       :    1;
        invoke_writeback                 :    1;
        rsvd_key_err                     :    1;
        rsvd                             :   18;
    }
}

header_type resp_rx_rkey_info_t {
    fields {
        va                               :   64;
        len                              :   32;
        dma_cmd_start_index              :    8;
        tbl_id                           :    3;
        acc_ctrl                         :    8;
        dma_cmdeop                       :    1;
        skip_pt                          :    1;
        rsvd_key_err                     :    1;
        user_key                         :    8;
        pad                              :   34;
    }
}

header_type resp_rx_sge_info_t {
    fields {
        sge_offset                       : 32;
        remaining_payload_bytes          : 16;
        dma_cmd_index                    :  8;
        num_sges                         :  2;
        dma_cmdeop                       :  1;
        invoke_writeback                 :  1;
        priv_oper_enable                 :  1;
        is_last_sge                      :  1;
        pad                              : 98;
    }
}

header_type resp_rx_rqcb1_write_back_info_t {
    fields {
        current_sge_offset               :   32;
        inv_r_key                        :   32;
        pad                              :   96;
    }
}

header_type resp_rx_rqcb_to_wqe_info_t {
    fields {
        rsvd                             :    6;
        recirc_path                      :    1;
        in_progress                      :    1;
        remaining_payload_bytes          :   16;
        curr_wqe_ptr                     :   64;
        current_sge_offset               :   32;
        current_sge_id                   :    8;
        num_valid_sges                   :    8;
        dma_cmd_index                    :    8;
        log_pmtu                         :    5;
        pad                              :   11;
    }
}

header_type resp_rx_lkey_to_pt_info_t {
    fields {
        pt_offset                        :   32;
        pt_bytes                         :   32;
        dma_cmd_start_index              :    8;
        sge_index                        :    3;
        log_page_size                    :    5;
        dma_cmdeop                       :    1;
        rsvd                             :    1;
        host_addr                        :    1;
        override_lif_vld                 :    1;
        override_lif                     :   12;
        pad                              :   64 ;
    }
}

header_type resp_rx_inv_rkey_info_t {
    fields {
        pad                              :  160;
    }
}

header_type resp_rx_to_stage_atomic_info_t {
    fields {
        rsqwqe_ptr                       :   64;
        pad                              :   64;
    }
}

header_type resp_rx_ecn_info_t {
    fields {
        p_key                            :   16;
        pad                              :  144;
    }
}

header_type resp_rx_rqcb_to_cq_info_t {
    fields {
        pad                              :  160;
    }
}

header_type resp_rx_rsq_backtrack_info_t {
    fields {
        log_pmtu                         :    5;
        read_or_atomic                   :    1;
        walk                             :    2;
        hi_index                         :   16;
        lo_index                         :   16;
        index                            :    8;
        log_rsq_size                     :    5;
        rsvd                             :    3;
        search_psn                       :   24;
        rsq_base_addr                    :   32;
        pad                              :   48;
    }
}

header_type resp_rx_dma_cmds_flit_t {
    fields {
        dma_cmd0                         :  128;
        dma_cmd1                         :  128;
        dma_cmd2                         :  128;
        dma_cmd3                         :  128;
    }
}

header_type resp_rx_cqcb_to_pt_info_t {
    fields {
        page_offset                      :   16;
        page_seg_offset                  :    8;
        cq_id                            :   24;
        fire_eqcb                        :    1;
        host_addr                        :    1;
        no_translate                     :    1;
        no_dma                           :    1;
        cqcb_addr                        :   34;
        pt_next_pg_index                 :   16;
        eqcb_addr                        :   34;
        report_error                     :    1;
        eqe_type                         :    3;
        eqe_code                         :    4;
        pad                              :   16;
    }
}

header_type resp_rx_rqcb_to_write_rkey_info_t {
    fields {
        va                               :   64;
        r_key                            :   32;
        len                              :   32;
        remaining_payload_bytes          :   16;
        load_reth                        :    1;
        incr_c_index                     :    1;
        pad                              :    2;
        priv_oper_enable                 :    1;
        rsvd                             :   11;
    }
}

header_type resp_rx_rqcb_to_rqcb1_info_t {
    fields {
        rsvd                             :    2;
        log_pmtu                         :    5;
        in_progress                      :    1;
        remaining_payload_bytes          :   16;
        curr_wqe_ptr                     :   64;
        current_sge_offset               :   32;
        current_sge_id                   :    8;
        num_sges                         :    8;
        pad                              :   24;
    }
}

header_type resp_rx_rqcb_to_pt_info_t {
    fields {
        in_progress                      :    1;
        page_seg_offset                  :    3;
        tbl_id                           :    3;
        rsvd                             :    1;
        page_offset                      :   16;
        remaining_payload_bytes          :   16;
        log_pmtu                         :    5;
        pad                              :  115;
    }
}

header_type phv_global_common_t {
    fields {
        lif                              :   11;
        qid                              :   24;
        qtype                            :    3;
        cb_addr                          :   25;
        pt_base_addr_page_id             :   22;
        log_num_pt_entries               :    5;
        pad                              :   22;
        _ud                              :    1;
        _ring_dbell                      :    1;
        _ack_req                         :    1;
        _completion                      :    1;
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

header_type resp_rx_to_stage_backtrack_info_t {
    fields {
        va                               :   64;
        r_key                            :   32;
        len                              :   32;
    }
}

header_type resp_rx_rsq_backtrack_adjust_info_t {
    fields {
        adjust_c_index                   :    1;
        adjust_p_index                   :    1;
        rsvd                             :    6;
        index                            :    8;
        pad                              :  144;
    }
}

header_type resp_rx_to_stage_stats_info_t {
    fields {
        pyld_bytes                       :   16;
        incr_mem_window_inv              :    1;
        incr_recirc_drop                 :    1;
        dup_wr_send                      :    1;
        dup_rd_atomic_bt                 :    1;
        dup_rd_atomic_drop               :    1;
        qp_err_disabled                  :    1;
        qp_err_dis_svc_type_err          :    1;
        qp_err_dis_pyld_len_err          :    1;
        qp_err_dis_last_pkt_len_err      :    1;
        qp_err_dis_pmtu_err              :    1;
        qp_err_dis_opcode_err            :    1;
        qp_err_dis_access_err            :    1;
        qp_err_dis_wr_only_zero_len_err  :    1;
        qp_err_dis_unaligned_atomic_va_err  :    1;
        qp_err_dis_dma_len_err           :    1;
        qp_err_dis_insuff_sge_err        :    1;
        qp_err_dis_max_sge_err           :    1;
        qp_err_dis_rsvd_key_err          :    1;
        qp_err_dis_key_state_err         :    1;
        qp_err_dis_key_pd_mismatch       :    1;
        qp_err_dis_key_acc_ctrl_err      :    1;
        qp_err_dis_user_key_err          :    1;
        qp_err_dis_key_va_err            :    1;
        qp_err_dis_inv_rkey_rsvd_key_err :    1;
        qp_err_dis_ineligible_mr_err     :    1;
        qp_err_dis_inv_rkey_state_err    :    1;
        qp_err_dis_type1_mw_inv_err      :    1;
        qp_err_dis_type2a_mw_qp_mismatch :    1;
        qp_err_dis_mr_mw_pd_mismatch     :    1;
        qp_err_dis_mr_state_invalid      :    1;
        qp_err_dis_mr_cookie_mismatch    :    1;
        qp_err_dis_feedback              :    1;
        qp_err_dis_table_error           :    1;
        qp_err_dis_phv_intrinsic_error   :    1;
        qp_err_dis_table_resp_error      :    1;
        qp_err_dis_rsvd                  :    2;
        max_recirc_cnt_err               :    1;
        lif_cqe_error_id_vld             :    1;
        lif_error_id_vld                 :    1;
        lif_error_id                     :    4;
        recirc_reason                    :    4;
        last_bth_opcode                  :    8;
        recirc_bth_psn                   :   24;
        np_ecn_marked_packets            :    1;
        rp_cnp_processed                 :    1;
        pad                              :   30;
    }
}

header_type resp_rx_to_stage_cqcb_info_t {
    fields {
        cqcb_base_addr_hi                :   24;
        log_num_cq_entries               :    4;
        bth_se                           :    1;
        async_event                      :    1;
        async_error_event                :    1;
        qp_state                         :    3;
        feedback                         :    1;
        pad                              :   93;
    }
}

header_type resp_rx_stats_info_t {
    fields {
        pad                              :  160;
    }
}

header_type resp_rx_to_stage_wb1_info_t {
    fields {
        curr_wqe_ptr                     :   64;
        my_token_id                      :    8;
        cqcb_base_addr_hi                :   24;
        log_num_cq_entries               :    4;
        in_progress                      :    1;
        incr_nxt_to_go_token_id          :    1;
        incr_c_index                     :    1;
        update_wqe_ptr                   :    1;
        update_num_sges                  :    1;
        soft_nak_or_dup                  :    1;
        feedback                         :    1;
        inv_rkey                         :    1;
        async_or_async_error_event       :    1;
        error_disable_qp                 :    1;
        send_sge_opt                     :    1;
        rsvd                             :    1;
        current_sge_id                   :    8;
        num_sges                         :    8;
    }
}

header_type resp_rx_to_stage_wqe_info_t {
    fields {
        ext_hdr_data                     :   69;
        inv_r_key                        :   32;
        spec_psn                         :   24;
        priv_oper_enable                 :   1;
        pad                              :   2;
    }
}

header_type resp_rx_to_stage_lkey_info_t {
    fields {
        pd                               :   32;
        mw_cookie                        :   32;
        rsvd_key_err                     :   1;
        user_key                         :   8;
        pad                              :   55;
    }
}

header_type resp_rx_rqcb_to_read_atomic_rkey_info_t {
    fields {
        va                               :   64;
        r_key                            :   32;
        len                              :   32;
        rsq_p_index                      :   16;
        skip_rsq_dbell                   :    1;
        read_or_atomic                   :    1;
        priv_oper_enable                 :    1;
        pad                              :   13;
    }
}

header_type resp_rx_cqcb_to_eq_info_t {
    fields {
        async_eq                         :  1;
        cmd_eop                          :  1;
        donot_reset_tbl_vld              :  1;
        pad                              :  157;
    }
}

header_type resp_rx_to_stage_recirc_info_t {
    fields {
        remaining_payload_bytes          :   16;
        curr_wqe_ptr                     :   64;
        current_sge_offset               :   32;
        current_sge_id                   :    8;
        num_sges                         :    8;
    }
}

header_type resp_rx_to_stage_rqpt_info_t {
    fields {
        send_sge_opt                     :    1;
        pad                              :  127;
    }
}

/**** header unions and scratch ****/

@pragma pa_header_union ingress app_header rdma_recirc rdma_bth rdma_bth_immeth rdma_bth_reth rdma_bth_reth_immeth rdma_bth_aeth rdma_bth_aeth_atomicaeth rdma_bth_atomiceth rdma_bth_ieth rdma_bth_deth rdma_bth_deth_immeth rdma_bth_xrceth rdma_bth_xrceth_immeth rdma_bth_xrceth_reth rdma_bth_xrceth_reth_immeth rdma_bth_xrceth_atomiceth rdma_bth_xrceth_ieth rdma_completion_feedback

metadata roce_recirc_header_t rdma_recirc;
metadata p4_to_p4plus_roce_bth_header_t rdma_bth;
metadata p4_to_p4plus_roce_bth_immeth_header_t rdma_bth_immeth;
metadata p4_to_p4plus_roce_bth_reth_header_t rdma_bth_reth;
metadata p4_to_p4plus_roce_bth_reth_immeth_header_t rdma_bth_reth_immeth;
metadata p4_to_p4plus_roce_bth_aeth_header_t rdma_bth_aeth;
metadata p4_to_p4plus_roce_bth_aeth_atomicaeth_header_t rdma_bth_aeth_atomicaeth;
metadata p4_to_p4plus_roce_bth_atomiceth_header_t rdma_bth_atomiceth;
metadata p4_to_p4plus_roce_bth_ieth_header_t rdma_bth_ieth;
metadata p4_to_p4plus_roce_bth_deth_header_t rdma_bth_deth;
metadata p4_to_p4plus_roce_bth_deth_immeth_header_t rdma_bth_deth_immeth;
metadata p4_to_p4plus_roce_bth_xrceth_header_t rdma_bth_xrceth;
metadata p4_to_p4plus_roce_bth_xrceth_immeth_header_t rdma_bth_xrceth_immeth;
metadata p4_to_p4plus_roce_bth_xrceth_reth_header_t rdma_bth_xrceth_reth;
metadata p4_to_p4plus_roce_bth_xrceth_reth_immeth_header_t rdma_bth_xrceth_reth_immeth;
metadata p4_to_p4plus_roce_bth_xrceth_atomiceth_header_t rdma_bth_xrceth_atomiceth;
metadata p4_to_p4plus_roce_bth_xrceth_ieth_header_t rdma_bth_xrceth_ieth;
metadata rdma_completion_feedback_header_t rdma_completion_feedback;


@pragma pa_header_union ingress ext_app_header rdma_bth_atomiceth_ext rdma_bth_xrceth_atomiceth_ext rdma_bth_xrceth_reth_immeth_ext rdma_bth_deth_immeth_ext rdma_bth_deth_ext

metadata p4_to_p4plus_roce_bth_atomiceth_ext_header_t rdma_bth_atomiceth_ext;
metadata p4_to_p4plus_roce_bth_xrceth_atomiceth_ext_header_t rdma_bth_xrceth_atomiceth_ext;
metadata p4_to_p4plus_roce_bth_xrceth_reth_immeth_ext_header_t rdma_bth_xrceth_reth_immeth_ext;
metadata p4_to_p4plus_roce_bth_deth_immeth_ext_header_t rdma_bth_deth_immeth_ext;
metadata p4_to_p4plus_roce_bth_deth_ext_header_t rdma_bth_deth_ext;

@pragma scratch_metadata
metadata roce_recirc_header_t rdma_recirc_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_header_t rdma_bth_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_immeth_header_t rdma_bth_immeth_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_reth_header_t rdma_bth_reth_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_reth_immeth_header_t rdma_bth_reth_immeth_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_aeth_header_t rdma_bth_aeth_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_aeth_atomicaeth_header_t rdma_bth_aeth_atomicaeth_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_atomiceth_header_t rdma_bth_atomiceth_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_ieth_header_t rdma_bth_ieth_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_deth_header_t rdma_bth_deth_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_deth_immeth_header_t rdma_bth_deth_immeth_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_xrceth_header_t rdma_bth_xrceth_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_xrceth_immeth_header_t rdma_bth_xrceth_immeth_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_xrceth_reth_header_t rdma_bth_xrceth_reth_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_xrceth_reth_immeth_header_t rdma_bth_xrceth_reth_immeth_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_xrceth_atomiceth_header_t rdma_bth_xrceth_atomiceth_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_xrceth_ieth_header_t rdma_bth_xrceth_ieth_scr;
@pragma scratch_metadata
metadata rdma_completion_feedback_header_t rdma_completion_feedback_scr;


//Extended headers
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_atomiceth_ext_header_t rdma_bth_atomiceth_ext_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_xrceth_atomiceth_ext_header_t rdma_bth_xrceth_atomiceth_ext_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_xrceth_reth_immeth_ext_header_t rdma_bth_xrceth_reth_immeth_ext_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_deth_immeth_ext_header_t rdma_bth_deth_immeth_ext_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_deth_ext_header_t rdma_bth_deth_ext_scr;

/**** global header unions ****/

@pragma pa_header_union ingress common_global
metadata phv_global_common_t phv_global_common;
@pragma scratch_metadata
metadata phv_global_common_t phv_global_common_scr;

/**** to stage header unions ****/

@pragma pa_header_union ingress to_stage_1
metadata resp_rx_to_stage_backtrack_info_t to_s1_backtrack_info;
@pragma scratch_metadata
metadata resp_rx_to_stage_backtrack_info_t to_s1_backtrack_info_scr;

@pragma pa_header_union ingress to_stage_1
metadata resp_rx_to_stage_atomic_info_t to_s1_atomic_info;
@pragma scratch_metadata
metadata resp_rx_to_stage_atomic_info_t to_s1_atomic_info_scr;

@pragma pa_header_union ingress to_stage_1
metadata resp_rx_to_stage_recirc_info_t to_s1_recirc_info;
@pragma scratch_metadata
metadata resp_rx_to_stage_recirc_info_t to_s1_recirc_info_scr;

@pragma pa_header_union ingress to_stage_1
metadata resp_rx_to_stage_rqpt_info_t to_s1_rqpt_info;
@pragma scratch_metadata
metadata resp_rx_to_stage_rqpt_info_t to_s1_rqpt_info_scr;

@pragma pa_header_union ingress to_stage_2
metadata resp_rx_to_stage_ext_hdr_info_t to_s2_ext_hdr_info;
@pragma scratch_metadata
metadata resp_rx_to_stage_ext_hdr_info_t to_s2_ext_hdr_info_scr;

@pragma pa_header_union ingress to_stage_2
metadata resp_rx_to_stage_wqe_info_t to_s2_wqe_info;
@pragma scratch_metadata
metadata resp_rx_to_stage_wqe_info_t to_s2_wqe_info_scr;

@pragma pa_header_union ingress to_stage_4
metadata resp_rx_to_stage_lkey_info_t to_s4_lkey_info;
@pragma scratch_metadata
metadata resp_rx_to_stage_lkey_info_t to_s4_lkey_info_scr;

@pragma pa_header_union ingress to_stage_5
metadata resp_rx_to_stage_wb1_info_t to_s5_wb1_info;
@pragma scratch_metadata
metadata resp_rx_to_stage_wb1_info_t to_s5_wb1_info_scr;

@pragma pa_header_union ingress to_stage_6
metadata resp_rx_to_stage_cqcb_info_t to_s6_cqcb_info;
@pragma scratch_metadata
metadata resp_rx_to_stage_cqcb_info_t to_s6_cqcb_info_scr;

@pragma pa_header_union ingress to_stage_7
metadata resp_rx_to_stage_stats_info_t to_s7_stats_info;
@pragma scratch_metadata
metadata resp_rx_to_stage_stats_info_t to_s7_stats_info_scr;

/**** stage to stage header unions ****/

@pragma pa_header_union ingress common_t0_s2s t0_s2s_rqcb_to_rqcb1_info t0_s2s_rsq_backtrack_info t0_s2s_rqcb_to_wqe_info t0_s2s_rsq_backtrack_adjust_info t0_s2s_lkey_to_pt_info t0_s2s_cqcb_to_eq_info t0_s2s_rqcb_to_pt_info t0_s2s_sge_info
metadata resp_rx_rqcb_to_rqcb1_info_t t0_s2s_rqcb_to_rqcb1_info;
@pragma scratch_metadata
metadata resp_rx_rqcb_to_rqcb1_info_t t0_s2s_rqcb_to_rqcb1_info_scr;

metadata resp_rx_rsq_backtrack_info_t t0_s2s_rsq_backtrack_info;
@pragma scratch_metadata
metadata resp_rx_rsq_backtrack_info_t t0_s2s_rsq_backtrack_info_scr;

metadata resp_rx_rqcb_to_wqe_info_t t0_s2s_rqcb_to_wqe_info;
@pragma scratch_metadata
metadata resp_rx_rqcb_to_wqe_info_t t0_s2s_rqcb_to_wqe_info_scr;

metadata resp_rx_rqcb_to_pt_info_t t0_s2s_rqcb_to_pt_info;
@pragma scratch_metadata
metadata resp_rx_rqcb_to_pt_info_t t0_s2s_rqcb_to_pt_info_scr;

metadata resp_rx_sge_info_t t0_s2s_sge_info;
@pragma scratch_metadata
metadata resp_rx_sge_info_t t0_s2s_sge_info_scr;

metadata resp_rx_rsq_backtrack_adjust_info_t t0_s2s_rsq_backtrack_adjust_info;
@pragma scratch_metadata
metadata resp_rx_rsq_backtrack_adjust_info_t t0_s2s_rsq_backtrack_adjust_info_scr;

metadata resp_rx_lkey_to_pt_info_t t0_s2s_lkey_to_pt_info;
@pragma scratch_metadata
metadata resp_rx_lkey_to_pt_info_t t0_s2s_lkey_to_pt_info_scr;

metadata resp_rx_cqcb_to_eq_info_t t0_s2s_cqcb_to_eq_info;
@pragma scratch_metadata
metadata resp_rx_cqcb_to_eq_info_t t0_s2s_cqcb_to_eq_info_scr;

@pragma pa_header_union ingress common_t1_s2s t1_s2s_rqcb_to_write_rkey_info t1_s2s_rqcb_to_read_atomic_rkey_info t1_s2s_key_info t1_s2s_rkey_info t1_s2s_cqcb_to_eq_info

metadata resp_rx_rqcb_to_write_rkey_info_t t1_s2s_rqcb_to_write_rkey_info;
@pragma scratch_metadata
metadata resp_rx_rqcb_to_write_rkey_info_t t1_s2s_rqcb_to_write_rkey_info_scr;

metadata resp_rx_rqcb_to_read_atomic_rkey_info_t t1_s2s_rqcb_to_read_atomic_rkey_info;
@pragma scratch_metadata
metadata resp_rx_rqcb_to_read_atomic_rkey_info_t t1_s2s_rqcb_to_read_atomic_rkey_info_scr;

metadata resp_rx_key_info_t t1_s2s_key_info;
@pragma scratch_metadata
metadata resp_rx_key_info_t t1_s2s_key_info_scr;

metadata resp_rx_rkey_info_t t1_s2s_rkey_info;
@pragma scratch_metadata
metadata resp_rx_rkey_info_t t1_s2s_rkey_info_scr;

metadata resp_rx_cqcb_to_eq_info_t t1_s2s_cqcb_to_eq_info;
@pragma scratch_metadata
metadata resp_rx_cqcb_to_eq_info_t t1_s2s_cqcb_to_eq_info_scr;

@pragma pa_header_union ingress common_t2_s2s t2_s2s_ecn_info t2_s2s_rqcb1_write_back_info t2_s2s_rqcb_to_cq_info t2_s2s_cqcb_to_pt_info
metadata resp_rx_ecn_info_t t2_s2s_ecn_info;
@pragma scratch_metadata
metadata resp_rx_ecn_info_t t2_s2s_ecn_info_scr;

metadata resp_rx_rqcb1_write_back_info_t t2_s2s_rqcb1_write_back_info;
@pragma scratch_metadata
metadata resp_rx_rqcb1_write_back_info_t t2_s2s_rqcb1_write_back_info_scr;

metadata resp_rx_rqcb_to_cq_info_t t2_s2s_rqcb_to_cq_info;
@pragma scratch_metadata
metadata resp_rx_rqcb_to_cq_info_t t2_s2s_rqcb_to_cq_info_scr;

metadata resp_rx_cqcb_to_pt_info_t t2_s2s_cqcb_to_pt_info;
@pragma scratch_metadata
metadata resp_rx_cqcb_to_pt_info_t t2_s2s_cqcb_to_pt_info_scr;

@pragma pa_header_union ingress common_t3_s2s t3_s2s_ecn_info t3_s2s_inv_rkey_info t3_s2s_stats_info
metadata resp_rx_ecn_info_t t3_s2s_ecn_info;
@pragma scratch_metadata
metadata resp_rx_ecn_info_t t3_s2s_ecn_info_scr;

metadata resp_rx_inv_rkey_info_t t3_s2s_inv_rkey_info;
@pragma scratch_metadata
metadata resp_rx_inv_rkey_info_t t3_s2s_inv_rkey_info_scr;

metadata resp_rx_stats_info_t t3_s2s_stats_info;
@pragma scratch_metadata
metadata resp_rx_stats_info_t t3_s2s_stats_info_scr;

/******************************************************************************
 * Action functions to generate k_struct and d_struct
 *
 * These action functions are currently only to generate the k+i and d structs
 * and do not implement any pseudo code
 *****************************************************************************/

/*
 * Stage 0 table 0 recirc action
 */
action rdma_stage0_recirc_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_recirc_scr.app_data0_1, rdma_recirc.app_data0_1);
    modify_field(rdma_recirc_scr.app_data0_2, rdma_recirc.app_data0_2);
    modify_field(rdma_recirc_scr.app_data1, rdma_recirc.app_data1);
    modify_field(rdma_recirc_scr.app_data2, rdma_recirc.app_data2);

    // recirc header bits
    modify_field(rdma_recirc_scr.token_id, rdma_recirc.token_id);
    modify_field(rdma_recirc_scr.recirc_reason, rdma_recirc.recirc_reason);
    modify_field(rdma_recirc_scr.recirc_iter_count, rdma_recirc.recirc_iter_count);
}

/*
 * Stage 0 table 0 bth action
 */
action rdma_stage0_bth_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from p4-to-p4plus-rdma-hdr
    modify_field(rdma_bth_scr.p4plus_app_id, rdma_bth.p4plus_app_id);
    modify_field(rdma_bth_scr.table0_valid, rdma_bth.table0_valid);
    modify_field(rdma_bth_scr.table1_valid, rdma_bth.table1_valid);
    modify_field(rdma_bth_scr.table2_valid, rdma_bth.table2_valid);
    modify_field(rdma_bth_scr.table3_valid, rdma_bth.table3_valid);
    modify_field(rdma_bth_scr.roce_opt_ts_valid, rdma_bth.roce_opt_ts_valid);
    modify_field(rdma_bth_scr.roce_opt_mss_valid, rdma_bth.roce_opt_mss_valid);
    modify_field(rdma_bth_scr.rdma_hdr_len, rdma_bth.rdma_hdr_len);
    modify_field(rdma_bth_scr.raw_flags, rdma_bth.raw_flags);
    modify_field(rdma_bth_scr.ecn, rdma_bth.ecn);
    modify_field(rdma_bth_scr.payload_len, rdma_bth.payload_len);
    modify_field(rdma_bth_scr.roce_opt_ts_value, rdma_bth.roce_opt_ts_value);
    modify_field(rdma_bth_scr.roce_opt_ts_echo, rdma_bth.roce_opt_ts_echo);
    modify_field(rdma_bth_scr.roce_opt_mss, rdma_bth.roce_opt_mss);
    modify_field(rdma_bth_scr.roce_int_recirc_hdr, rdma_bth.roce_int_recirc_hdr);

    // bth params
    modify_field(rdma_bth_scr.bth_opcode, rdma_bth.bth_opcode);
    modify_field(rdma_bth_scr.bth_se, rdma_bth.bth_se);
    modify_field(rdma_bth_scr.bth_m, rdma_bth.bth_m);
    modify_field(rdma_bth_scr.bth_pad, rdma_bth.bth_pad);
    modify_field(rdma_bth_scr.bth_tver, rdma_bth.bth_tver);
    modify_field(rdma_bth_scr.bth_pkey, rdma_bth.bth_pkey);
    modify_field(rdma_bth_scr.bth_f, rdma_bth.bth_f);
    modify_field(rdma_bth_scr.bth_b, rdma_bth.bth_b);
    modify_field(rdma_bth_scr.bth_rsvd1, rdma_bth.bth_rsvd1);
    modify_field(rdma_bth_scr.bth_dst_qp, rdma_bth.bth_dst_qp);
    modify_field(rdma_bth_scr.bth_a, rdma_bth.bth_a);
    modify_field(rdma_bth_scr.bth_rsvd2, rdma_bth.bth_rsvd2);
    modify_field(rdma_bth_scr.bth_psn, rdma_bth.bth_psn);

}

/*
 * Stage 0 table 0 bth immeth action
 */
action rdma_stage0_bth_immeth_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_bth_immeth_scr.common_header_bits, rdma_bth_immeth.common_header_bits);

    // bth bits
    modify_field(rdma_bth_immeth_scr.bth_header_bits, rdma_bth_immeth.bth_header_bits);

    // immeth params
    modify_field(rdma_bth_immeth_scr.immeth_data, rdma_bth_immeth.immeth_data);
}


/*
 * Stage 0 table 0 bth reth action
 */
action rdma_stage0_bth_reth_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_bth_reth_scr.common_header_bits, rdma_bth_reth.common_header_bits);

    // bth bits
    modify_field(rdma_bth_reth_scr.bth_header_bits, rdma_bth_reth.bth_header_bits);

    // reth params
    modify_field(rdma_bth_reth_scr.reth_va_1, rdma_bth_reth.reth_va_1);
    modify_field(rdma_bth_reth_scr.reth_va_2, rdma_bth_reth.reth_va_2);
    modify_field(rdma_bth_reth_scr.reth_r_key, rdma_bth_reth.reth_r_key);
    modify_field(rdma_bth_reth_scr.reth_dma_len1, rdma_bth_reth.reth_dma_len1);
    modify_field(rdma_bth_reth_scr.reth_dma_len2, rdma_bth_reth.reth_dma_len2);
}


/*
 * Stage 0 table 0 bth reth immeth action
 */
action rdma_stage0_bth_reth_immeth_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_bth_reth_immeth_scr.common_header_bits, rdma_bth_reth_immeth.common_header_bits);

    // bth bits
    modify_field(rdma_bth_reth_immeth_scr.bth_header_bits, rdma_bth_reth_immeth.bth_header_bits);

    // reth bits
    modify_field(rdma_bth_reth_immeth_scr.reth_header_bits_1, rdma_bth_reth_immeth.reth_header_bits_1);
    modify_field(rdma_bth_reth_immeth_scr.reth_header_bits_2, rdma_bth_reth_immeth.reth_header_bits_2);

    // reth_immeth params
    modify_field(rdma_bth_reth_immeth_scr.immeth_data, rdma_bth_reth_immeth.immeth_data);
}


/*
 * Stage 0 table 0 bth aeth action
 */
action rdma_stage0_bth_aeth_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_bth_aeth_scr.common_header_bits, rdma_bth_aeth.common_header_bits);

    // bth bits
    modify_field(rdma_bth_aeth_scr.bth_header_bits, rdma_bth_aeth.bth_header_bits);

    // aeth params
    modify_field(rdma_bth_aeth_scr.aeth_syndrome, rdma_bth_aeth.aeth_syndrome);
    modify_field(rdma_bth_aeth_scr.aeth_msn, rdma_bth_aeth.aeth_msn);
}


/*
 * Stage 0 table 0 bth aeth_atomicaeth action
 */
action rdma_stage0_bth_aeth_atomicaeth_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_bth_aeth_atomicaeth_scr.common_header_bits, rdma_bth_aeth_atomicaeth.common_header_bits);

    // bth bits
    modify_field(rdma_bth_aeth_atomicaeth_scr.bth_header_bits, rdma_bth_aeth_atomicaeth.bth_header_bits);

    // aeth bits
    modify_field(rdma_bth_aeth_atomicaeth_scr.aeth_header_bits, rdma_bth_aeth_atomicaeth.aeth_header_bits);

    // atomicaeth params
    modify_field(rdma_bth_aeth_atomicaeth_scr.atomicaeth_orig_data, rdma_bth_aeth_atomicaeth.atomicaeth_orig_data);
}

/*
 * Stage 0 table 0 bth atomiceth action
 */
action rdma_stage0_bth_atomiceth_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_bth_atomiceth_scr.common_header_bits, rdma_bth_atomiceth.common_header_bits);

    // bth bits
    modify_field(rdma_bth_atomiceth_scr.bth_header_bits, rdma_bth_atomiceth.bth_header_bits);

    // atomiceth params
    modify_field(rdma_bth_atomiceth_scr.atomiceth_va_1, rdma_bth_atomiceth.atomiceth_va_1);
    modify_field(rdma_bth_atomiceth_scr.atomiceth_va_2, rdma_bth_atomiceth.atomiceth_va_2);
    modify_field(rdma_bth_atomiceth_scr.atomiceth_r_key, rdma_bth_atomiceth.atomiceth_r_key);
    modify_field(rdma_bth_atomiceth_scr.atomiceth_swap_or_add_data, rdma_bth_atomiceth.atomiceth_swap_or_add_data);
    //modify_field(rdma_bth_atomiceth_scr.atomiceth_cmp_data, rdma_bth_atomiceth.atomiceth_cmp_data);
}

/*
 * Stage 0 rdma_params table ext bth atomiceth action
 */
action rdma_stage0_ext_bth_atomiceth_action () {
    // k + i for stage 0

    modify_field(rdma_bth_atomiceth_ext_scr.atomiceth_cmp_data, rdma_bth_atomiceth_ext.atomiceth_cmp_data);
}


/*
 * Stage 0 table 0 bth ieth action
 */
action rdma_stage0_bth_ieth_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_bth_ieth_scr.common_header_bits, rdma_bth_ieth.common_header_bits);

    // bth bits
    modify_field(rdma_bth_ieth_scr.bth_header_bits, rdma_bth_ieth.bth_header_bits);

    // ieth params
    modify_field(rdma_bth_ieth_scr.ieth_r_key, rdma_bth_ieth.ieth_r_key);
}


/*
 * Stage 0 table 0 bth deth action
 */
action rdma_stage0_bth_deth_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_bth_deth_scr.common_header_bits, rdma_bth_deth.common_header_bits);

    // bth bits
    modify_field(rdma_bth_deth_scr.bth_header_bits, rdma_bth_deth.bth_header_bits);

    // deth params
    modify_field(rdma_bth_deth_scr.deth_q_key, rdma_bth_deth.deth_q_key);
    modify_field(rdma_bth_deth_scr.deth_rsvd, rdma_bth_deth.deth_rsvd);
    modify_field(rdma_bth_deth_scr.deth_src_qp, rdma_bth_deth.deth_src_qp);
    modify_field(rdma_bth_deth_scr.smac, rdma_bth_deth.smac);
    modify_field(rdma_bth_deth_scr.dmac, rdma_bth_deth.dmac);
    //modify_field(rdma_bth_deth_scr.vlan_ethtype, rdma_bth_deth.vlan_ethtype);
    //modify_field(rdma_bth_deth_scr.vlan, rdma_bth_deth.vlan);
    //modify_field(rdma_bth_deth_scr.ethtype, rdma_bth_deth.ethtype);
}


/*
 * Stage 0 table 0 bth deth immeth action
 */
action rdma_stage0_ext_bth_deth_action () {
    // k + i for stage 0

    modify_field(rdma_bth_deth_ext_scr.vlan_ethtype, rdma_bth_deth_ext.vlan_ethtype);
    modify_field(rdma_bth_deth_ext_scr.vlan, rdma_bth_deth_ext.vlan);
    modify_field(rdma_bth_deth_ext_scr.ethtype, rdma_bth_deth_ext.ethtype);
}


/*
 * Stage 0 table 0 bth deth immeth action
 */
action rdma_stage0_bth_deth_immeth_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_bth_deth_immeth_scr.common_header_bits, rdma_bth_deth_immeth.common_header_bits);

    // bth bits
    modify_field(rdma_bth_deth_immeth_scr.bth_header_bits, rdma_bth_deth_immeth.bth_header_bits);

    // deth bits
    modify_field(rdma_bth_deth_immeth_scr.deth_header_bits_1, rdma_bth_deth_immeth.deth_header_bits_1);
    modify_field(rdma_bth_deth_immeth_scr.deth_header_bits_2, rdma_bth_deth_immeth.deth_header_bits_2);

    // deth_immeth params
    modify_field(rdma_bth_deth_immeth_scr.immeth_data, rdma_bth_deth_immeth.immeth_data);
    modify_field(rdma_bth_deth_immeth_scr.smac_1, rdma_bth_deth_immeth.smac_1);
    //modify_field(rdma_bth_deth_immeth_scr.smac_2, rdma_bth_deth_immeth.smac_2);
    //modify_field(rdma_bth_deth_immeth_scr.vlan_ethtype, rdma_bth_deth_immeth.vlan_ethtype);
    //modify_field(rdma_bth_deth_immeth_scr.vlan, rdma_bth_deth_immeth.vlan);
    //modify_field(rdma_bth_deth_immeth_scr.ethtype, rdma_bth_deth_immeth.ethtype);
}

/*
 * Stage 0 table 0 bth deth immeth action
 */
action rdma_stage0_ext_bth_deth_immeth_action () {
    // k + i for stage 0

    modify_field(rdma_bth_deth_immeth_ext_scr.smac_2, rdma_bth_deth_immeth_ext.smac_2);
    modify_field(rdma_bth_deth_immeth_ext_scr.vlan_ethtype, rdma_bth_deth_immeth_ext.vlan_ethtype);
    modify_field(rdma_bth_deth_immeth_ext_scr.vlan, rdma_bth_deth_immeth_ext.vlan);
    modify_field(rdma_bth_deth_immeth_ext_scr.ethtype, rdma_bth_deth_immeth_ext.ethtype);
}



/*
 * Stage 0 table 0 bth xrceth action
 */
action rdma_stage0_bth_xrceth_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_bth_xrceth_scr.common_header_bits, rdma_bth_xrceth.common_header_bits);

    // bth bits
    modify_field(rdma_bth_xrceth_scr.bth_header_bits, rdma_bth_xrceth.bth_header_bits);

    // xrceth params
    modify_field(rdma_bth_xrceth_scr.xrceth_rsvd, rdma_bth_xrceth.xrceth_rsvd);
    modify_field(rdma_bth_xrceth_scr.xrceth_xrcsrq, rdma_bth_xrceth.xrceth_xrcsrq);
}


/*
 * Stage 0 table 0 bth xrceth immeth action
 */
action rdma_stage0_bth_xrceth_immeth_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_bth_xrceth_immeth_scr.common_header_bits, rdma_bth_xrceth_immeth.common_header_bits);

    // bth bits
    modify_field(rdma_bth_xrceth_immeth_scr.bth_header_bits, rdma_bth_xrceth_immeth.bth_header_bits);

    // xrceth bits
    modify_field(rdma_bth_xrceth_immeth_scr.xrceth_header_bits, rdma_bth_xrceth_immeth.xrceth_header_bits);

    // xrceth_immeth params
    modify_field(rdma_bth_xrceth_immeth_scr.immeth_data, rdma_bth_xrceth_immeth.immeth_data);
}


/*
 * Stage 0 table 0 bth xrceth reth action
 */
action rdma_stage0_bth_xrceth_reth_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_bth_xrceth_reth_scr.common_header_bits, rdma_bth_xrceth_reth.common_header_bits);

    // bth bits
    modify_field(rdma_bth_xrceth_reth_scr.bth_header_bits, rdma_bth_xrceth_reth.bth_header_bits);

    // xrceth bits
    modify_field(rdma_bth_xrceth_reth_scr.xrceth_header_bits, rdma_bth_xrceth_reth.xrceth_header_bits);

    // reth params
    modify_field(rdma_bth_xrceth_reth_scr.reth_va, rdma_bth_xrceth_reth.reth_va);
    modify_field(rdma_bth_xrceth_reth_scr.reth_r_key, rdma_bth_xrceth_reth.reth_r_key);
    modify_field(rdma_bth_xrceth_reth_scr.reth_dma_len, rdma_bth_xrceth_reth.reth_dma_len);
}


/*
 * Stage 0 table 0 bth reth immeth action
 */
action rdma_stage0_bth_xrceth_reth_immeth_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_bth_xrceth_reth_immeth_scr.common_header_bits, rdma_bth_xrceth_reth_immeth.common_header_bits);

    // bth bits
    modify_field(rdma_bth_xrceth_reth_immeth_scr.bth_header_bits, rdma_bth_xrceth_reth_immeth.bth_header_bits);

    // xrceth bits
    modify_field(rdma_bth_xrceth_reth_immeth_scr.xrceth_header_bits, rdma_bth_xrceth_reth_immeth.xrceth_header_bits);

    // reth bits
    modify_field(rdma_bth_xrceth_reth_immeth_scr.reth_header_bits, rdma_bth_xrceth_reth_immeth.reth_header_bits);

    // reth_immeth params
    //modify_field(rdma_bth_xrceth_reth_immeth_scr.immeth_data, rdma_bth_xrceth_reth_immeth.immeth_data);
}


/*
 * Stage 0 table 0 ext bth reth immeth action
 */
action rdma_stage0_ext_bth_xrceth_reth_immeth_action () {
    // k + i for stage 0

    // reth_immeth params
    modify_field(rdma_bth_xrceth_reth_immeth_ext_scr.immeth_data, rdma_bth_xrceth_reth_immeth_ext.immeth_data);
}

/*
 * Stage 0 table 0 bth atomiceth action
 */
action rdma_stage0_bth_xrceth_atomiceth_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_bth_xrceth_atomiceth_scr.common_header_bits, rdma_bth_xrceth_atomiceth.common_header_bits);

    // bth bits
    modify_field(rdma_bth_xrceth_atomiceth_scr.bth_header_bits, rdma_bth_xrceth_atomiceth.bth_header_bits);

    // xrceth bits
    modify_field(rdma_bth_xrceth_atomiceth_scr.xrceth_header_bits, rdma_bth_xrceth_atomiceth.xrceth_header_bits);

    // atomiceth params
    modify_field(rdma_bth_xrceth_atomiceth_scr.atomiceth_va, rdma_bth_xrceth_atomiceth.atomiceth_va);
    modify_field(rdma_bth_xrceth_atomiceth_scr.atomiceth_r_key, rdma_bth_xrceth_atomiceth.atomiceth_r_key);
    modify_field(rdma_bth_xrceth_atomiceth_scr.atomiceth_swap_or_add_data_1, rdma_bth_xrceth_atomiceth.atomiceth_swap_or_add_data_1);
    //modify_field(rdma_bth_xrceth_atomiceth_scr.atomiceth_swap_or_add_data_2, rdma_bth_xrceth_atomiceth.atomiceth_swap_or_add_data_2);
    //modify_field(rdma_bth_xrceth_atomiceth_scr.atomiceth_cmp_data, rdma_bth_xrceth_atomiceth.atomiceth_cmp_data);
}

/*
 * Stage 0 rdma_params table ext bth atomiceth action
 */
action rdma_stage0_ext_bth_xrceth_atomiceth_action () {
    // k + i for stage 0

    modify_field(rdma_bth_xrceth_atomiceth_ext_scr.atomiceth_swap_or_add_data_2, rdma_bth_xrceth_atomiceth_ext.atomiceth_swap_or_add_data_2);
    modify_field(rdma_bth_xrceth_atomiceth_ext_scr.atomiceth_cmp_data, rdma_bth_xrceth_atomiceth_ext.atomiceth_cmp_data);
}

/*
 * Stage 0 table 0 bth ieth action
 */
action rdma_stage0_bth_xrceth_ieth_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_bth_xrceth_ieth_scr.common_header_bits, rdma_bth_xrceth_ieth.common_header_bits);

    // bth bits
    modify_field(rdma_bth_xrceth_ieth_scr.bth_header_bits, rdma_bth_xrceth_ieth.bth_header_bits);

    // xrceth bits
    modify_field(rdma_bth_xrceth_ieth_scr.xrceth_header_bits, rdma_bth_xrceth_ieth.xrceth_header_bits);

    // ieth params
    modify_field(rdma_bth_xrceth_ieth_scr.ieth_r_key, rdma_bth_xrceth_ieth.ieth_r_key);
}

/*
 * Stage 0 table 0 ud_feedback action
 */
action rdma_stage0_completion_feedback_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_completion_feedback_scr.common_header_bits, rdma_completion_feedback.common_header_bits);

    // ud_feedback_header bits
    modify_field(rdma_completion_feedback_scr.feedback_type, rdma_completion_feedback.feedback_type);
    modify_field(rdma_completion_feedback_scr.wrid, rdma_completion_feedback.wrid);
    modify_field(rdma_completion_feedback_scr.status, rdma_completion_feedback.status);
    modify_field(rdma_completion_feedback_scr.error, rdma_completion_feedback.error);
}


action resp_rx_atomic_resource_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_atomic_info_scr.rsqwqe_ptr, to_s1_atomic_info.rsqwqe_ptr);
    modify_field(to_s1_atomic_info_scr.pad, to_s1_atomic_info.pad);

    // stage to stage
    modify_field(t1_s2s_rqcb_to_read_atomic_rkey_info_scr.va, t1_s2s_rqcb_to_read_atomic_rkey_info.va);
    modify_field(t1_s2s_rqcb_to_read_atomic_rkey_info_scr.len, t1_s2s_rqcb_to_read_atomic_rkey_info.len);
    modify_field(t1_s2s_rqcb_to_read_atomic_rkey_info_scr.r_key, t1_s2s_rqcb_to_read_atomic_rkey_info.r_key);
    modify_field(t1_s2s_rqcb_to_read_atomic_rkey_info_scr.rsq_p_index, t1_s2s_rqcb_to_read_atomic_rkey_info.rsq_p_index);
    modify_field(t1_s2s_rqcb_to_read_atomic_rkey_info_scr.skip_rsq_dbell, t1_s2s_rqcb_to_read_atomic_rkey_info.skip_rsq_dbell);
    modify_field(t1_s2s_rqcb_to_read_atomic_rkey_info_scr.read_or_atomic, t1_s2s_rqcb_to_read_atomic_rkey_info.read_or_atomic);
    modify_field(t1_s2s_rqcb_to_read_atomic_rkey_info_scr.priv_oper_enable, t1_s2s_rqcb_to_read_atomic_rkey_info.priv_oper_enable);
    modify_field(t1_s2s_rqcb_to_read_atomic_rkey_info_scr.pad, t1_s2s_rqcb_to_read_atomic_rkey_info.pad);

}
action resp_rx_dcqcn_cnp_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t2_s2s_ecn_info_scr.p_key, t2_s2s_ecn_info.p_key);
    modify_field(t2_s2s_ecn_info_scr.pad, t2_s2s_ecn_info.pad);

}
action resp_rx_dcqcn_ecn_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t3_s2s_ecn_info_scr.p_key, t3_s2s_ecn_info.p_key);
    modify_field(t3_s2s_ecn_info_scr.pad, t3_s2s_ecn_info.pad);

}

action resp_rx_rome_pkt_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t3_s2s_ecn_info_scr.p_key, t3_s2s_ecn_info.p_key);
    modify_field(t3_s2s_ecn_info_scr.pad, t3_s2s_ecn_info.pad);
}

action resp_rx_rome_cnp_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage

}

action resp_rx_rqpt_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_rqpt_info_scr.send_sge_opt, to_s1_rqpt_info.send_sge_opt);
    modify_field(to_s1_rqpt_info_scr.pad, to_s1_rqpt_info.pad);

    // stage to stage
    modify_field(t0_s2s_rqcb_to_pt_info_scr.in_progress, t0_s2s_rqcb_to_pt_info.in_progress);
    modify_field(t0_s2s_rqcb_to_pt_info_scr.page_seg_offset, t0_s2s_rqcb_to_pt_info.page_seg_offset);
    modify_field(t0_s2s_rqcb_to_pt_info_scr.tbl_id, t0_s2s_rqcb_to_pt_info.tbl_id);
    modify_field(t0_s2s_rqcb_to_pt_info_scr.rsvd, t0_s2s_rqcb_to_pt_info.rsvd);
    modify_field(t0_s2s_rqcb_to_pt_info_scr.page_offset, t0_s2s_rqcb_to_pt_info.page_offset);
    modify_field(t0_s2s_rqcb_to_pt_info_scr.remaining_payload_bytes, t0_s2s_rqcb_to_pt_info.remaining_payload_bytes);
    modify_field(t0_s2s_rqcb_to_pt_info_scr.log_pmtu, t0_s2s_rqcb_to_pt_info.log_pmtu);
    modify_field(t0_s2s_rqcb_to_pt_info_scr.pad, t0_s2s_rqcb_to_pt_info.pad);

}

action resp_rx_dummy_rqpt_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_rqpt_info_scr.send_sge_opt, to_s1_rqpt_info.send_sge_opt);
    modify_field(to_s1_rqpt_info_scr.pad, to_s1_rqpt_info.pad);

    // stage to stage
    modify_field(t0_s2s_rqcb_to_wqe_info_scr.rsvd, t0_s2s_rqcb_to_wqe_info.rsvd);
    modify_field(t0_s2s_rqcb_to_wqe_info_scr.recirc_path, t0_s2s_rqcb_to_wqe_info.recirc_path);
    modify_field(t0_s2s_rqcb_to_wqe_info_scr.in_progress, t0_s2s_rqcb_to_wqe_info.in_progress);
    modify_field(t0_s2s_rqcb_to_wqe_info_scr.remaining_payload_bytes, t0_s2s_rqcb_to_wqe_info.remaining_payload_bytes);
    modify_field(t0_s2s_rqcb_to_wqe_info_scr.curr_wqe_ptr, t0_s2s_rqcb_to_wqe_info.curr_wqe_ptr);
    modify_field(t0_s2s_rqcb_to_wqe_info_scr.current_sge_offset, t0_s2s_rqcb_to_wqe_info.current_sge_offset);
    modify_field(t0_s2s_rqcb_to_wqe_info_scr.current_sge_id, t0_s2s_rqcb_to_wqe_info.current_sge_id);
    modify_field(t0_s2s_rqcb_to_wqe_info_scr.num_valid_sges, t0_s2s_rqcb_to_wqe_info.num_valid_sges);
    modify_field(t0_s2s_rqcb_to_wqe_info_scr.dma_cmd_index, t0_s2s_rqcb_to_wqe_info.dma_cmd_index);
    modify_field(t0_s2s_rqcb_to_wqe_info_scr.log_pmtu, t0_s2s_rqcb_to_wqe_info.log_pmtu);
    modify_field(t0_s2s_rqcb_to_wqe_info_scr.pad, t0_s2s_rqcb_to_wqe_info.pad);

}

action resp_rx_eqcb_process_t0 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t0_s2s_cqcb_to_eq_info_scr.async_eq, t0_s2s_cqcb_to_eq_info.async_eq);
    modify_field(t0_s2s_cqcb_to_eq_info_scr.cmd_eop, t0_s2s_cqcb_to_eq_info.cmd_eop);
    modify_field(t0_s2s_cqcb_to_eq_info_scr.donot_reset_tbl_vld, t0_s2s_cqcb_to_eq_info.donot_reset_tbl_vld);
    modify_field(t0_s2s_cqcb_to_eq_info_scr.pad, t0_s2s_cqcb_to_eq_info.pad);

}
action resp_rx_eqcb_process_t1 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t1_s2s_cqcb_to_eq_info_scr.async_eq, t1_s2s_cqcb_to_eq_info.async_eq);
    modify_field(t1_s2s_cqcb_to_eq_info_scr.cmd_eop, t1_s2s_cqcb_to_eq_info.cmd_eop);
    modify_field(t1_s2s_cqcb_to_eq_info_scr.donot_reset_tbl_vld, t1_s2s_cqcb_to_eq_info.donot_reset_tbl_vld);
    modify_field(t1_s2s_cqcb_to_eq_info_scr.pad, t1_s2s_cqcb_to_eq_info.pad);

}
action resp_rx_inv_rkey_validate_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_lkey_info_scr.pd, to_s4_lkey_info.pd);
    modify_field(to_s4_lkey_info_scr.mw_cookie, to_s4_lkey_info.mw_cookie);
    modify_field(to_s4_lkey_info_scr.rsvd_key_err, to_s4_lkey_info.rsvd_key_err);
    modify_field(to_s4_lkey_info_scr.user_key, to_s4_lkey_info.user_key);
    modify_field(to_s4_lkey_info_scr.pad, to_s4_lkey_info.pad);

    // stage to stage
    modify_field(t3_s2s_inv_rkey_info_scr.pad, t3_s2s_inv_rkey_info.pad);

}
action resp_rx_inv_rkey_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
}
action resp_rx_ptseg_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t0_s2s_lkey_to_pt_info_scr.pt_offset, t0_s2s_lkey_to_pt_info.pt_offset);
    modify_field(t0_s2s_lkey_to_pt_info_scr.pt_bytes, t0_s2s_lkey_to_pt_info.pt_bytes);
    modify_field(t0_s2s_lkey_to_pt_info_scr.dma_cmd_start_index, t0_s2s_lkey_to_pt_info.dma_cmd_start_index);
    modify_field(t0_s2s_lkey_to_pt_info_scr.sge_index, t0_s2s_lkey_to_pt_info.sge_index);
    modify_field(t0_s2s_lkey_to_pt_info_scr.log_page_size, t0_s2s_lkey_to_pt_info.log_page_size);
    modify_field(t0_s2s_lkey_to_pt_info_scr.dma_cmdeop, t0_s2s_lkey_to_pt_info.dma_cmdeop);
    modify_field(t0_s2s_lkey_to_pt_info_scr.rsvd, t0_s2s_lkey_to_pt_info.rsvd);
    modify_field(t0_s2s_lkey_to_pt_info_scr.host_addr, t0_s2s_lkey_to_pt_info.host_addr);
    modify_field(t0_s2s_lkey_to_pt_info_scr.override_lif_vld, t0_s2s_lkey_to_pt_info.override_lif_vld);
    modify_field(t0_s2s_lkey_to_pt_info_scr.override_lif, t0_s2s_lkey_to_pt_info.override_lif);
    modify_field(t0_s2s_lkey_to_pt_info_scr.pad, t0_s2s_lkey_to_pt_info.pad);

}
action resp_rx_read_mpu_only_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t1_s2s_rqcb_to_read_atomic_rkey_info_scr.va, t1_s2s_rqcb_to_read_atomic_rkey_info.va);
    modify_field(t1_s2s_rqcb_to_read_atomic_rkey_info_scr.len, t1_s2s_rqcb_to_read_atomic_rkey_info.len);
    modify_field(t1_s2s_rqcb_to_read_atomic_rkey_info_scr.r_key, t1_s2s_rqcb_to_read_atomic_rkey_info.r_key);
    modify_field(t1_s2s_rqcb_to_read_atomic_rkey_info_scr.rsq_p_index, t1_s2s_rqcb_to_read_atomic_rkey_info.rsq_p_index);
    modify_field(t1_s2s_rqcb_to_read_atomic_rkey_info_scr.skip_rsq_dbell, t1_s2s_rqcb_to_read_atomic_rkey_info.skip_rsq_dbell);
    modify_field(t1_s2s_rqcb_to_read_atomic_rkey_info_scr.read_or_atomic, t1_s2s_rqcb_to_read_atomic_rkey_info.read_or_atomic);
    modify_field(t1_s2s_rqcb_to_read_atomic_rkey_info_scr.pad, t1_s2s_rqcb_to_read_atomic_rkey_info.pad);

}
action resp_rx_rqcb3_in_progress_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t0_s2s_rqcb_to_rqcb1_info_scr.rsvd, t0_s2s_rqcb_to_rqcb1_info.rsvd);
    modify_field(t0_s2s_rqcb_to_rqcb1_info_scr.log_pmtu, t0_s2s_rqcb_to_rqcb1_info.log_pmtu);
    modify_field(t0_s2s_rqcb_to_rqcb1_info_scr.in_progress, t0_s2s_rqcb_to_rqcb1_info.in_progress);
    modify_field(t0_s2s_rqcb_to_rqcb1_info_scr.remaining_payload_bytes, t0_s2s_rqcb_to_rqcb1_info.remaining_payload_bytes);
    modify_field(t0_s2s_rqcb_to_rqcb1_info_scr.curr_wqe_ptr, t0_s2s_rqcb_to_rqcb1_info.curr_wqe_ptr);
    modify_field(t0_s2s_rqcb_to_rqcb1_info_scr.current_sge_offset, t0_s2s_rqcb_to_rqcb1_info.current_sge_offset);
    modify_field(t0_s2s_rqcb_to_rqcb1_info_scr.current_sge_id, t0_s2s_rqcb_to_rqcb1_info.current_sge_id);
    modify_field(t0_s2s_rqcb_to_rqcb1_info_scr.num_sges, t0_s2s_rqcb_to_rqcb1_info.num_sges);
    modify_field(t0_s2s_rqcb_to_rqcb1_info_scr.pad, t0_s2s_rqcb_to_rqcb1_info.pad);

}
action resp_rx_rqcb1_recirc_sge_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_recirc_info_scr.remaining_payload_bytes, to_s1_recirc_info.remaining_payload_bytes);
    modify_field(to_s1_recirc_info_scr.curr_wqe_ptr, to_s1_recirc_info.curr_wqe_ptr);
    modify_field(to_s1_recirc_info_scr.current_sge_offset, to_s1_recirc_info.current_sge_offset);
    modify_field(to_s1_recirc_info_scr.current_sge_id, to_s1_recirc_info.current_sge_id);
    modify_field(to_s1_recirc_info_scr.num_sges, to_s1_recirc_info.num_sges);

    // stage to stage
    modify_field(t0_s2s_rqcb_to_rqcb1_info_scr.rsvd, t0_s2s_rqcb_to_rqcb1_info.rsvd);
    modify_field(t0_s2s_rqcb_to_rqcb1_info_scr.in_progress, t0_s2s_rqcb_to_rqcb1_info.in_progress);
    modify_field(t0_s2s_rqcb_to_rqcb1_info_scr.remaining_payload_bytes, t0_s2s_rqcb_to_rqcb1_info.remaining_payload_bytes);
    modify_field(t0_s2s_rqcb_to_rqcb1_info_scr.curr_wqe_ptr, t0_s2s_rqcb_to_rqcb1_info.curr_wqe_ptr);
    modify_field(t0_s2s_rqcb_to_rqcb1_info_scr.current_sge_offset, t0_s2s_rqcb_to_rqcb1_info.current_sge_offset);
    modify_field(t0_s2s_rqcb_to_rqcb1_info_scr.current_sge_id, t0_s2s_rqcb_to_rqcb1_info.current_sge_id);
    modify_field(t0_s2s_rqcb_to_rqcb1_info_scr.num_sges, t0_s2s_rqcb_to_rqcb1_info.num_sges);
    modify_field(t0_s2s_rqcb_to_rqcb1_info_scr.pad, t0_s2s_rqcb_to_rqcb1_info.pad);

}
action resp_rx_rqlkey_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_lkey_info_scr.pd, to_s4_lkey_info.pd);
    modify_field(to_s4_lkey_info_scr.mw_cookie, to_s4_lkey_info.mw_cookie);
    modify_field(to_s4_lkey_info_scr.rsvd_key_err, to_s4_lkey_info.rsvd_key_err);
    modify_field(to_s4_lkey_info_scr.pad, to_s4_lkey_info.pad);

    // stage to stage
    modify_field(t1_s2s_key_info_scr.va, t1_s2s_key_info.va);
    modify_field(t1_s2s_key_info_scr.current_sge_offset, t1_s2s_key_info.current_sge_offset);
    modify_field(t1_s2s_key_info_scr.len, t1_s2s_key_info.len);
    modify_field(t1_s2s_key_info_scr.dma_cmd_start_index, t1_s2s_key_info.dma_cmd_start_index);
    modify_field(t1_s2s_key_info_scr.tbl_id, t1_s2s_key_info.tbl_id);
    modify_field(t1_s2s_key_info_scr.dma_cmdeop, t1_s2s_key_info.dma_cmdeop);
    modify_field(t1_s2s_key_info_scr.invoke_writeback, t1_s2s_key_info.invoke_writeback);
    modify_field(t1_s2s_key_info_scr.rsvd_key_err, t1_s2s_key_info.rsvd_key_err);
    modify_field(t1_s2s_key_info_scr.rsvd, t1_s2s_key_info.rsvd);

}

action resp_rx_rqlkey_mr_cookie_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_lkey_info_scr.pd, to_s4_lkey_info.pd);
    modify_field(to_s4_lkey_info_scr.mw_cookie, to_s4_lkey_info.mw_cookie);
    modify_field(to_s4_lkey_info_scr.pad, to_s4_lkey_info.pad);

    // stage to stage

}

action resp_rx_rqrkey_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s2_ext_hdr_info_scr.ext_hdr_data, to_s2_ext_hdr_info.ext_hdr_data);
    modify_field(to_s2_ext_hdr_info_scr.rsvd, to_s2_ext_hdr_info.rsvd);
    modify_field(to_s2_ext_hdr_info_scr.pd, to_s2_ext_hdr_info.pd);

    // stage to stage
    modify_field(t1_s2s_rkey_info_scr.va, t1_s2s_rkey_info.va);
    modify_field(t1_s2s_rkey_info_scr.len, t1_s2s_rkey_info.len);
    modify_field(t1_s2s_rkey_info_scr.dma_cmd_start_index, t1_s2s_rkey_info.dma_cmd_start_index);
    modify_field(t1_s2s_rkey_info_scr.tbl_id, t1_s2s_rkey_info.tbl_id);
    modify_field(t1_s2s_rkey_info_scr.acc_ctrl, t1_s2s_rkey_info.acc_ctrl);
    modify_field(t1_s2s_rkey_info_scr.dma_cmdeop, t1_s2s_rkey_info.dma_cmdeop);
    modify_field(t1_s2s_rkey_info_scr.skip_pt, t1_s2s_rkey_info.skip_pt);
    modify_field(t1_s2s_rkey_info_scr.rsvd_key_err, t1_s2s_rkey_info.rsvd_key_err);
    modify_field(t1_s2s_rkey_info_scr.user_key, t1_s2s_rkey_info.user_key);
    modify_field(t1_s2s_rkey_info_scr.pad, t1_s2s_rkey_info.pad);

}

action resp_rx_rsq_backtrack_adjust_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t0_s2s_rsq_backtrack_adjust_info_scr.adjust_c_index, t0_s2s_rsq_backtrack_adjust_info.adjust_c_index);
    modify_field(t0_s2s_rsq_backtrack_adjust_info_scr.adjust_p_index, t0_s2s_rsq_backtrack_adjust_info.adjust_p_index);
    modify_field(t0_s2s_rsq_backtrack_adjust_info_scr.rsvd, t0_s2s_rsq_backtrack_adjust_info.rsvd);
    modify_field(t0_s2s_rsq_backtrack_adjust_info_scr.index, t0_s2s_rsq_backtrack_adjust_info.index);
    modify_field(t0_s2s_rsq_backtrack_adjust_info_scr.pad, t0_s2s_rsq_backtrack_adjust_info.pad);

}
action resp_rx_rsq_backtrack_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_backtrack_info_scr.va, to_s1_backtrack_info.va);
    modify_field(to_s1_backtrack_info_scr.r_key, to_s1_backtrack_info.r_key);
    modify_field(to_s1_backtrack_info_scr.len, to_s1_backtrack_info.len);

    // stage to stage
    modify_field(t0_s2s_rsq_backtrack_info_scr.log_pmtu, t0_s2s_rsq_backtrack_info.log_pmtu);
    modify_field(t0_s2s_rsq_backtrack_info_scr.read_or_atomic, t0_s2s_rsq_backtrack_info.read_or_atomic);
    modify_field(t0_s2s_rsq_backtrack_info_scr.walk, t0_s2s_rsq_backtrack_info.walk);
    modify_field(t0_s2s_rsq_backtrack_info_scr.hi_index, t0_s2s_rsq_backtrack_info.hi_index);
    modify_field(t0_s2s_rsq_backtrack_info_scr.lo_index, t0_s2s_rsq_backtrack_info.lo_index);
    modify_field(t0_s2s_rsq_backtrack_info_scr.index, t0_s2s_rsq_backtrack_info.index);
    modify_field(t0_s2s_rsq_backtrack_info_scr.log_rsq_size, t0_s2s_rsq_backtrack_info.log_rsq_size);
    modify_field(t0_s2s_rsq_backtrack_info_scr.rsvd, t0_s2s_rsq_backtrack_info.rsvd);
    modify_field(t0_s2s_rsq_backtrack_info_scr.search_psn, t0_s2s_rsq_backtrack_info.search_psn);
    modify_field(t0_s2s_rsq_backtrack_info_scr.rsq_base_addr, t0_s2s_rsq_backtrack_info.rsq_base_addr);
    modify_field(t0_s2s_rsq_backtrack_info_scr.pad, t0_s2s_rsq_backtrack_info.pad);

}
action resp_rx_stats_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s7_stats_info_scr.pyld_bytes, to_s7_stats_info.pyld_bytes);
    modify_field(to_s7_stats_info_scr.incr_mem_window_inv, to_s7_stats_info.incr_mem_window_inv);
    modify_field(to_s7_stats_info_scr.incr_recirc_drop, to_s7_stats_info.incr_recirc_drop);
    modify_field(to_s7_stats_info_scr.dup_wr_send, to_s7_stats_info.dup_wr_send);
    modify_field(to_s7_stats_info_scr.dup_rd_atomic_bt, to_s7_stats_info.dup_rd_atomic_bt);
    modify_field(to_s7_stats_info_scr.dup_rd_atomic_drop, to_s7_stats_info.dup_rd_atomic_drop);
    modify_field(to_s7_stats_info_scr.qp_err_disabled, to_s7_stats_info.qp_err_disabled);
    modify_field(to_s7_stats_info_scr.qp_err_dis_svc_type_err, to_s7_stats_info.qp_err_dis_svc_type_err);
    modify_field(to_s7_stats_info_scr.qp_err_dis_pyld_len_err, to_s7_stats_info.qp_err_dis_pyld_len_err);
    modify_field(to_s7_stats_info_scr.qp_err_dis_last_pkt_len_err, to_s7_stats_info.qp_err_dis_last_pkt_len_err);
    modify_field(to_s7_stats_info_scr.qp_err_dis_pmtu_err, to_s7_stats_info.qp_err_dis_pmtu_err);
    modify_field(to_s7_stats_info_scr.qp_err_dis_opcode_err, to_s7_stats_info.qp_err_dis_opcode_err);
    modify_field(to_s7_stats_info_scr.qp_err_dis_access_err, to_s7_stats_info.qp_err_dis_access_err);
    modify_field(to_s7_stats_info_scr.qp_err_dis_wr_only_zero_len_err, to_s7_stats_info.qp_err_dis_wr_only_zero_len_err);
    modify_field(to_s7_stats_info_scr.qp_err_dis_unaligned_atomic_va_err, to_s7_stats_info.qp_err_dis_unaligned_atomic_va_err);
    modify_field(to_s7_stats_info_scr.qp_err_dis_dma_len_err, to_s7_stats_info.qp_err_dis_dma_len_err);
    modify_field(to_s7_stats_info_scr.qp_err_dis_insuff_sge_err, to_s7_stats_info.qp_err_dis_insuff_sge_err);
    modify_field(to_s7_stats_info_scr.qp_err_dis_max_sge_err, to_s7_stats_info.qp_err_dis_max_sge_err);
    modify_field(to_s7_stats_info_scr.qp_err_dis_inv_rkey_rsvd_key_err, to_s7_stats_info.qp_err_dis_inv_rkey_rsvd_key_err);
    modify_field(to_s7_stats_info_scr.qp_err_dis_ineligible_mr_err, to_s7_stats_info.qp_err_dis_ineligible_mr_err);
    modify_field(to_s7_stats_info_scr.qp_err_dis_inv_rkey_state_err, to_s7_stats_info.qp_err_dis_inv_rkey_state_err);
    modify_field(to_s7_stats_info_scr.qp_err_dis_type1_mw_inv_err, to_s7_stats_info.qp_err_dis_type1_mw_inv_err);
    modify_field(to_s7_stats_info_scr.qp_err_dis_type2a_mw_qp_mismatch, to_s7_stats_info.qp_err_dis_type2a_mw_qp_mismatch);
    modify_field(to_s7_stats_info_scr.qp_err_dis_mr_mw_pd_mismatch, to_s7_stats_info.qp_err_dis_mr_mw_pd_mismatch);
    modify_field(to_s7_stats_info_scr.qp_err_dis_mr_state_invalid, to_s7_stats_info.qp_err_dis_mr_state_invalid);
    modify_field(to_s7_stats_info_scr.qp_err_dis_mr_cookie_mismatch, to_s7_stats_info.qp_err_dis_mr_cookie_mismatch);
    modify_field(to_s7_stats_info_scr.qp_err_dis_rsvd_key_err, to_s7_stats_info.qp_err_dis_rsvd_key_err);
    modify_field(to_s7_stats_info_scr.qp_err_dis_key_state_err, to_s7_stats_info.qp_err_dis_key_state_err);
    modify_field(to_s7_stats_info_scr.qp_err_dis_key_pd_mismatch, to_s7_stats_info.qp_err_dis_key_pd_mismatch);
    modify_field(to_s7_stats_info_scr.qp_err_dis_key_acc_ctrl_err, to_s7_stats_info.qp_err_dis_key_acc_ctrl_err);
    modify_field(to_s7_stats_info_scr.qp_err_dis_user_key_err, to_s7_stats_info.qp_err_dis_user_key_err);
    modify_field(to_s7_stats_info_scr.qp_err_dis_key_va_err, to_s7_stats_info.qp_err_dis_key_va_err);
    modify_field(to_s7_stats_info_scr.qp_err_dis_feedback, to_s7_stats_info.qp_err_dis_feedback);
    modify_field(to_s7_stats_info_scr.qp_err_dis_table_error, to_s7_stats_info.qp_err_dis_table_error);
    modify_field(to_s7_stats_info_scr.qp_err_dis_phv_intrinsic_error, to_s7_stats_info.qp_err_dis_phv_intrinsic_error);
    modify_field(to_s7_stats_info_scr.qp_err_dis_table_resp_error, to_s7_stats_info.qp_err_dis_table_resp_error);
    modify_field(to_s7_stats_info_scr.qp_err_dis_rsvd, to_s7_stats_info.qp_err_dis_rsvd);
    modify_field(to_s7_stats_info_scr.max_recirc_cnt_err, to_s7_stats_info.max_recirc_cnt_err);
    modify_field(to_s7_stats_info_scr.lif_cqe_error_id_vld, to_s7_stats_info.lif_cqe_error_id_vld);
    modify_field(to_s7_stats_info_scr.lif_error_id_vld, to_s7_stats_info.lif_error_id_vld);
    modify_field(to_s7_stats_info_scr.lif_error_id, to_s7_stats_info.lif_error_id);
    modify_field(to_s7_stats_info_scr.recirc_reason, to_s7_stats_info.recirc_reason);
    modify_field(to_s7_stats_info_scr.last_bth_opcode, to_s7_stats_info.last_bth_opcode);
    modify_field(to_s7_stats_info_scr.recirc_bth_psn, to_s7_stats_info.recirc_bth_psn);
    modify_field(to_s7_stats_info_scr.np_ecn_marked_packets, to_s7_stats_info.np_ecn_marked_packets);
    modify_field(to_s7_stats_info_scr.rp_cnp_processed, to_s7_stats_info.rp_cnp_processed);
    modify_field(to_s7_stats_info_scr.pad, to_s7_stats_info.pad);

    // stage to stage
    modify_field(t3_s2s_stats_info_scr.pad, t3_s2s_stats_info.pad);

}
action resp_rx_write_dummy_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t1_s2s_rqcb_to_write_rkey_info_scr.va, t1_s2s_rqcb_to_write_rkey_info.va);
    modify_field(t1_s2s_rqcb_to_write_rkey_info_scr.len, t1_s2s_rqcb_to_write_rkey_info.len);
    modify_field(t1_s2s_rqcb_to_write_rkey_info_scr.r_key, t1_s2s_rqcb_to_write_rkey_info.r_key);
    modify_field(t1_s2s_rqcb_to_write_rkey_info_scr.remaining_payload_bytes, t1_s2s_rqcb_to_write_rkey_info.remaining_payload_bytes);
    modify_field(t1_s2s_rqcb_to_write_rkey_info_scr.load_reth, t1_s2s_rqcb_to_write_rkey_info.load_reth);
    modify_field(t1_s2s_rqcb_to_write_rkey_info_scr.incr_c_index, t1_s2s_rqcb_to_write_rkey_info.incr_c_index);
    modify_field(t1_s2s_rqcb_to_write_rkey_info_scr.pad, t1_s2s_rqcb_to_write_rkey_info.pad);
    modify_field(t1_s2s_rqcb_to_write_rkey_info_scr.priv_oper_enable, t1_s2s_rqcb_to_write_rkey_info.priv_oper_enable);
    modify_field(t1_s2s_rqcb_to_write_rkey_info_scr.rsvd, t1_s2s_rqcb_to_write_rkey_info.rsvd);

}
action resp_rx_cqcb_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s6_cqcb_info_scr.cqcb_base_addr_hi, to_s6_cqcb_info.cqcb_base_addr_hi);
    modify_field(to_s6_cqcb_info_scr.log_num_cq_entries, to_s6_cqcb_info.log_num_cq_entries);
    modify_field(to_s6_cqcb_info_scr.bth_se, to_s6_cqcb_info.bth_se);
    modify_field(to_s6_cqcb_info_scr.async_event, to_s6_cqcb_info.async_event);
    modify_field(to_s6_cqcb_info_scr.async_error_event, to_s6_cqcb_info.async_error_event);
    modify_field(to_s6_cqcb_info_scr.qp_state, to_s6_cqcb_info.qp_state);
    modify_field(to_s6_cqcb_info_scr.feedback, to_s6_cqcb_info.feedback);
    modify_field(to_s6_cqcb_info_scr.pad, to_s6_cqcb_info.pad);


    // stage to stage
    modify_field(t2_s2s_rqcb_to_cq_info_scr.pad, t2_s2s_rqcb_to_cq_info.pad);

}
action resp_rx_cqpt_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t2_s2s_cqcb_to_pt_info_scr.page_offset, t2_s2s_cqcb_to_pt_info.page_offset);
    modify_field(t2_s2s_cqcb_to_pt_info_scr.page_seg_offset, t2_s2s_cqcb_to_pt_info.page_seg_offset);
    modify_field(t2_s2s_cqcb_to_pt_info_scr.cq_id, t2_s2s_cqcb_to_pt_info.cq_id);
    modify_field(t2_s2s_cqcb_to_pt_info_scr.fire_eqcb, t2_s2s_cqcb_to_pt_info.fire_eqcb);
    modify_field(t2_s2s_cqcb_to_pt_info_scr.host_addr, t2_s2s_cqcb_to_pt_info.host_addr);
    modify_field(t2_s2s_cqcb_to_pt_info_scr.no_translate, t2_s2s_cqcb_to_pt_info.no_translate);
    modify_field(t2_s2s_cqcb_to_pt_info_scr.no_dma, t2_s2s_cqcb_to_pt_info.no_dma);
    modify_field(t2_s2s_cqcb_to_pt_info_scr.cqcb_addr, t2_s2s_cqcb_to_pt_info.cqcb_addr);
    modify_field(t2_s2s_cqcb_to_pt_info_scr.pt_next_pg_index, t2_s2s_cqcb_to_pt_info.pt_next_pg_index);
    modify_field(t2_s2s_cqcb_to_pt_info_scr.eqcb_addr, t2_s2s_cqcb_to_pt_info.eqcb_addr);
    modify_field(t2_s2s_cqcb_to_pt_info_scr.report_error, t2_s2s_cqcb_to_pt_info.report_error);
    modify_field(t2_s2s_cqcb_to_pt_info_scr.eqe_type, t2_s2s_cqcb_to_pt_info.eqe_type);
    modify_field(t2_s2s_cqcb_to_pt_info_scr.eqe_code, t2_s2s_cqcb_to_pt_info.eqe_code);
    modify_field(t2_s2s_cqcb_to_pt_info_scr.pad, t2_s2s_cqcb_to_pt_info.pad);

}

action resp_rx_recirc_mpu_only_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage

}

action resp_rx_rqcb_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage

}
action resp_rx_rqcb1_write_back_mpu_only_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
}
action resp_rx_rqwqe_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s2_wqe_info_scr.ext_hdr_data, to_s2_wqe_info.ext_hdr_data);
    modify_field(to_s2_wqe_info_scr.inv_r_key, to_s2_wqe_info.inv_r_key);
    modify_field(to_s2_wqe_info_scr.spec_psn, to_s2_wqe_info.spec_psn);
    modify_field(to_s2_wqe_info_scr.priv_oper_enable, to_s2_wqe_info.priv_oper_enable);
    modify_field(to_s2_wqe_info_scr.pad, to_s2_wqe_info.pad);

    // stage to stage
    modify_field(t0_s2s_rqcb_to_wqe_info_scr.rsvd, t0_s2s_rqcb_to_wqe_info.rsvd);
    modify_field(t0_s2s_rqcb_to_wqe_info_scr.recirc_path, t0_s2s_rqcb_to_wqe_info.recirc_path);
    modify_field(t0_s2s_rqcb_to_wqe_info_scr.in_progress, t0_s2s_rqcb_to_wqe_info.in_progress);
    modify_field(t0_s2s_rqcb_to_wqe_info_scr.remaining_payload_bytes, t0_s2s_rqcb_to_wqe_info.remaining_payload_bytes);
    modify_field(t0_s2s_rqcb_to_wqe_info_scr.curr_wqe_ptr, t0_s2s_rqcb_to_wqe_info.curr_wqe_ptr);
    modify_field(t0_s2s_rqcb_to_wqe_info_scr.current_sge_offset, t0_s2s_rqcb_to_wqe_info.current_sge_offset);
    modify_field(t0_s2s_rqcb_to_wqe_info_scr.current_sge_id, t0_s2s_rqcb_to_wqe_info.current_sge_id);
    modify_field(t0_s2s_rqcb_to_wqe_info_scr.num_valid_sges, t0_s2s_rqcb_to_wqe_info.num_valid_sges);
    modify_field(t0_s2s_rqcb_to_wqe_info_scr.dma_cmd_index, t0_s2s_rqcb_to_wqe_info.dma_cmd_index);
    modify_field(t0_s2s_rqcb_to_wqe_info_scr.log_pmtu, t0_s2s_rqcb_to_wqe_info.log_pmtu);
    modify_field(t0_s2s_rqcb_to_wqe_info_scr.pad, t0_s2s_rqcb_to_wqe_info.pad);

}

action resp_rx_rqwqe_opt_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s2_wqe_info_scr.ext_hdr_data, to_s2_wqe_info.ext_hdr_data);
    modify_field(to_s2_wqe_info_scr.inv_r_key, to_s2_wqe_info.inv_r_key);
    modify_field(to_s2_wqe_info_scr.spec_psn, to_s2_wqe_info.spec_psn);
    modify_field(to_s2_wqe_info_scr.priv_oper_enable, to_s2_wqe_info.priv_oper_enable);
    modify_field(to_s2_wqe_info_scr.pad, to_s2_wqe_info.pad);

    // stage to stage
    modify_field(t0_s2s_rqcb_to_wqe_info_scr.rsvd, t0_s2s_rqcb_to_wqe_info.rsvd);
    modify_field(t0_s2s_rqcb_to_wqe_info_scr.recirc_path, t0_s2s_rqcb_to_wqe_info.recirc_path);
    modify_field(t0_s2s_rqcb_to_wqe_info_scr.in_progress, t0_s2s_rqcb_to_wqe_info.in_progress);
    modify_field(t0_s2s_rqcb_to_wqe_info_scr.remaining_payload_bytes, t0_s2s_rqcb_to_wqe_info.remaining_payload_bytes);
    modify_field(t0_s2s_rqcb_to_wqe_info_scr.curr_wqe_ptr, t0_s2s_rqcb_to_wqe_info.curr_wqe_ptr);
    modify_field(t0_s2s_rqcb_to_wqe_info_scr.current_sge_offset, t0_s2s_rqcb_to_wqe_info.current_sge_offset);
    modify_field(t0_s2s_rqcb_to_wqe_info_scr.current_sge_id, t0_s2s_rqcb_to_wqe_info.current_sge_id);
    modify_field(t0_s2s_rqcb_to_wqe_info_scr.num_valid_sges, t0_s2s_rqcb_to_wqe_info.num_valid_sges);
    modify_field(t0_s2s_rqcb_to_wqe_info_scr.dma_cmd_index, t0_s2s_rqcb_to_wqe_info.dma_cmd_index);
    modify_field(t0_s2s_rqcb_to_wqe_info_scr.log_pmtu, t0_s2s_rqcb_to_wqe_info.log_pmtu);
    modify_field(t0_s2s_rqcb_to_wqe_info_scr.pad, t0_s2s_rqcb_to_wqe_info.pad);

}

action resp_rx_rqwqe_wrid_process () {
    // from ki global
    GENERATE_GLOBAL_K
}

action resp_rx_rqsge_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t0_s2s_sge_info_scr.sge_offset, t0_s2s_sge_info.sge_offset);
    modify_field(t0_s2s_sge_info_scr.remaining_payload_bytes, t0_s2s_sge_info.remaining_payload_bytes);
    modify_field(t0_s2s_sge_info_scr.dma_cmd_index, t0_s2s_sge_info.dma_cmd_index);
    modify_field(t0_s2s_sge_info_scr.num_sges, t0_s2s_sge_info.num_sges);
    modify_field(t0_s2s_sge_info_scr.dma_cmdeop, t0_s2s_sge_info.dma_cmdeop);
    modify_field(t0_s2s_sge_info_scr.invoke_writeback, t0_s2s_sge_info.invoke_writeback);
    modify_field(t0_s2s_sge_info_scr.priv_oper_enable, t0_s2s_sge_info.priv_oper_enable);
    modify_field(t0_s2s_sge_info_scr.is_last_sge, t0_s2s_sge_info.is_last_sge);
    modify_field(t0_s2s_sge_info_scr.pad, t0_s2s_sge_info.pad);

}

action resp_rx_rqcb1_write_back_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s5_wb1_info_scr.curr_wqe_ptr, to_s5_wb1_info.curr_wqe_ptr);
    modify_field(to_s5_wb1_info_scr.my_token_id, to_s5_wb1_info.my_token_id);
    modify_field(to_s5_wb1_info_scr.cqcb_base_addr_hi, to_s5_wb1_info.cqcb_base_addr_hi);
    modify_field(to_s5_wb1_info_scr.log_num_cq_entries, to_s5_wb1_info.log_num_cq_entries);
    modify_field(to_s5_wb1_info_scr.in_progress, to_s5_wb1_info.in_progress);
    modify_field(to_s5_wb1_info_scr.incr_nxt_to_go_token_id, to_s5_wb1_info.incr_nxt_to_go_token_id);
    modify_field(to_s5_wb1_info_scr.incr_c_index, to_s5_wb1_info.incr_c_index);
    modify_field(to_s5_wb1_info_scr.update_wqe_ptr, to_s5_wb1_info.update_wqe_ptr);
    modify_field(to_s5_wb1_info_scr.update_num_sges, to_s5_wb1_info.update_num_sges);
    modify_field(to_s5_wb1_info_scr.soft_nak_or_dup, to_s5_wb1_info.soft_nak_or_dup);
    modify_field(to_s5_wb1_info_scr.feedback, to_s5_wb1_info.feedback);
    modify_field(to_s5_wb1_info_scr.inv_rkey, to_s5_wb1_info.inv_rkey);
    modify_field(to_s5_wb1_info_scr.async_or_async_error_event, to_s5_wb1_info.async_or_async_error_event);
    modify_field(to_s5_wb1_info_scr.error_disable_qp, to_s5_wb1_info.error_disable_qp);
    modify_field(to_s5_wb1_info_scr.send_sge_opt, to_s5_wb1_info.send_sge_opt);
    modify_field(to_s5_wb1_info_scr.rsvd, to_s5_wb1_info.rsvd);
    modify_field(to_s5_wb1_info_scr.current_sge_id, to_s5_wb1_info.current_sge_id);
    modify_field(to_s5_wb1_info_scr.num_sges, to_s5_wb1_info.num_sges);

    // stage to stage
    modify_field(t2_s2s_rqcb1_write_back_info_scr.current_sge_offset, t2_s2s_rqcb1_write_back_info.current_sge_offset);
    modify_field(t2_s2s_rqcb1_write_back_info_scr.inv_r_key, t2_s2s_rqcb1_write_back_info.inv_r_key);
    modify_field(t2_s2s_rqcb1_write_back_info_scr.pad, t2_s2s_rqcb1_write_back_info.pad);

}

action resp_rx_rqcb1_write_back_err_process () {
    // from ki global
    GENERATE_GLOBAL_K
}

action resp_rx_phv_drop_mpu_only_process () {
    // from ki global
    GENERATE_GLOBAL_K
}
