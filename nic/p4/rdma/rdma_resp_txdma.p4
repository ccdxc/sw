/***********************************************************************/
/* rdma_resp_txdma.p4 */
/***********************************************************************/

#include "../common-p4+/common_txdma_dummy.p4"

#define tx_table_s0_t0 rdma_stage0_table
#define tx_table_s0_t0_action  rdma_stage0_table_action
#define tx_table_s0_t0_action1 rdma_stage0_recirc_action

/**** table declarations ****/

#define tx_table_s0_t1 resp_tx_s0_t1
#define tx_table_s0_t2 resp_tx_s0_t2
#define tx_table_s0_t3 resp_tx_s0_t3

#define tx_table_s1_t0 resp_tx_s1_t0
#define tx_table_s1_t1 resp_tx_s1_t1
#define tx_table_s1_t2 resp_tx_s1_t2
#define tx_table_s1_t3 resp_tx_s1_t3

#define tx_table_s2_t0 resp_tx_s2_t0
#define tx_table_s2_t1 resp_tx_s2_t1
#define tx_table_s2_t2 resp_tx_s2_t2
#define tx_table_s2_t3 resp_tx_s2_t3

#define tx_table_s3_t0 resp_tx_s3_t0
#define tx_table_s3_t1 resp_tx_s3_t1
#define tx_table_s3_t2 resp_tx_s3_t2
#define tx_table_s3_t3 resp_tx_s3_t3

#define tx_table_s4_t0 resp_tx_s4_t0
#define tx_table_s4_t1 resp_tx_s4_t1
#define tx_table_s4_t2 resp_tx_s4_t2
#define tx_table_s4_t3 resp_tx_s4_t3

#define tx_table_s5_t0 resp_tx_s5_t0
#define tx_table_s5_t1 resp_tx_s5_t1
#define tx_table_s5_t2 resp_tx_s5_t2
#define tx_table_s5_t3 resp_tx_s5_t3

#define tx_table_s6_t0 resp_tx_s6_t0
#define tx_table_s6_t1 resp_tx_s6_t1
#define tx_table_s6_t2 resp_tx_s6_t2
#define tx_table_s6_t3 resp_tx_s6_t3

#define tx_table_s7_t0 resp_tx_s7_t0
#define tx_table_s7_t1 resp_tx_s7_t1
#define tx_table_s7_t2 resp_tx_s7_t2
#define tx_table_s7_t3 resp_tx_s7_t3


#define tx_table_s0_t0_action resp_tx_rqcb_process

#define tx_table_s1_t0_action   resp_tx_rqcb2_process
#define tx_table_s1_t0_action1  resp_tx_ack_process
#define tx_table_s1_t0_action2  resp_tx_rsq_backtrack_adjust_process
#define tx_table_s1_t0_action3  resp_tx_rqcb2_bt_process

#define tx_table_s1_t1_action   resp_tx_dcqcn_config_load_process
#define tx_table_s1_t2_action   resp_tx_setup_checkout_process

#define tx_table_s2_t0_action   resp_tx_rsqwqe_process
#define tx_table_s2_t0_action1  resp_tx_rsqwqe_bt_process

#define tx_table_s2_t2_action   resp_tx_pre_checkout_process

#define tx_table_s3_t0_action resp_tx_rsqrkey_process

#define tx_table_s3_t1_action   resp_tx_rqcb0_bt_write_back_process

#define tx_table_s3_t2_action   resp_tx_checkout_process
#define tx_table_s3_t2_action1  resp_tx_rqprefetch_mpu_only_process

#define tx_table_s4_t0_action   resp_tx_rsqptseg_process
#define tx_table_s4_t0_action1  resp_tx_dcqcn_rate_process
#define tx_table_s4_t0_action2  resp_tx_dcqcn_timer_process

#define tx_table_s4_t1_action   resp_tx_dcqcn_enforce_process

#define tx_table_s4_t2_action   resp_tx_rqprefetch_process

#define tx_table_s4_t3_action   resp_tx_rsqrkey_mr_cookie_process

#define tx_table_s5_t1_action   resp_tx_rqcb0_write_back_process

#define tx_table_s5_t2_action   resp_tx_rqpt_mpu_only_process

#define tx_table_s6_t2_action   resp_tx_rqpt_process

#define tx_table_s7_t3_action   resp_tx_stats_process

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
    modify_field(phv_global_common_scr.ah_base_addr_page_id, phv_global_common.ah_base_addr_page_id);\
    modify_field(phv_global_common_scr._error_disable_qp, phv_global_common._error_disable_qp);\
    modify_field(phv_global_common_scr._only, phv_global_common._only);\
    modify_field(phv_global_common_scr._first, phv_global_common._first);\
    modify_field(phv_global_common_scr._middle, phv_global_common._middle);\
    modify_field(phv_global_common_scr._last, phv_global_common._last);\
    modify_field(phv_global_common_scr._read_resp, phv_global_common._read_resp);\
    modify_field(phv_global_common_scr._atomic_resp, phv_global_common._atomic_resp);\
    modify_field(phv_global_common_scr._ack, phv_global_common._ack);\
    modify_field(phv_global_common_scr._rsvd_flags, phv_global_common._rsvd_flags);

/**** header definitions ****/

header_type resp_tx_to_stage_rqcb1_wb_info_t {
    fields {
        new_c_index                      :   16;
        ack_nak_process                  :    1;
        flush_rq                         :    1;
        rsvd                             :  110;
    }
}

header_type resp_tx_rkey_to_ptseg_info_t {
    fields {
        pt_seg_offset                    :   32;
        pt_seg_bytes                     :   32;
        dma_cmd_start_index              :    8;
        log_page_size                    :    5;
        tbl_id                           :    2;
        host_addr                        :    1;
        pad                              :   80;
    }
}

header_type resp_tx_rqcb_to_ack_info_t {
    fields {
        header_template_addr             :   32;
        pad                              :  128;
    }
}

header_type resp_tx_to_stage_rsq_rkey_info_t {
    fields {
        dcqcn_cb_addr                    :   34;
        congestion_mgmt_type             :    2;
        rsvd                             :   92;
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
        ah_base_addr_page_id             :   22;
        _rsvd_flags                      :    8;
        _atomic_resp                     :    1;
        _read_resp                       :    1;
        _ack                             :    1;
        _only                            :    1;
        _last                            :    1;
        _middle                          :    1;
        _first                           :    1;
        _error_disable_qp                :    1;
    }
}

header_type resp_tx_rqcb_to_rqcb2_info_t {
    fields {
        rsqwqe_addr                      :   64;
        curr_read_rsp_psn                :   24;
        log_pmtu                         :    5;
        serv_type                        :    3;
        header_template_addr             :   32;
        header_template_size             :    8;
        pad                              :   24;
    }
}

header_type resp_tx_to_stage_wqe_info_t {
    fields {
        dcqcn_cb_addr                    :   34;
        rsvd                             :   94;
    }
}

header_type resp_tx_rsq_backtrack_adjust_info_t {
    fields {
        adjust_rsq_c_index               :   16;
        rsq_bt_p_index                   :   16;
        pad                              :  128;
    }
}

header_type resp_tx_s1_info_t {
    fields {
        rsvd                             :  128;
    }
}

header_type resp_tx_rsqrkey_to_rkey_cookie_info_t {
    fields {
        mw_cookie                        :   32;
        pad                              :  128;
    }
}

header_type resp_tx_rqcb0_write_back_info_t {
    fields {
        curr_read_rsp_psn                :   24;
        read_rsp_in_progress             :    1;
        rate_enforce_failed              :    1;
        pad                              :  134;
    }
}

header_type resp_tx_rqcb_to_dcqcn_cfg_info_t {
    fields {
        dcqcn_config_base                :   34;
        dcqcn_config_id                  :    4;
        dcqcn_rate_timer_toggle          :    1;
        pad                              :  121;
    }
}

header_type resp_tx_cfg_to_dcqcn_info_t {
    fields {
        alpha_timer_interval             :   32;
        timer_exp_thr                    :   16;
        g_val                            :   16;
        ai_rate                          :   18;
        hai_rate                         :   18;
        threshold                        :    5;
        byte_reset                       :   32;
        clamp_tgt_rate                   :    2;
        pad                              :   21;
    }
}

header_type resp_tx_to_stage_dcqcn_info_t {
    fields {
        dcqcn_cb_addr                    :   34;
        congestion_mgmt_type             :    2;
        packet_len                       :   14;
        resp_rl_failure                  :    1;
        rsvd                             :   45;
        pd                               :   32;
    }
}

header_type resp_tx_to_stage_dcqcn_rate_timer_t {
    fields {
        min_qp_rate                      :   32;
        min_qp_target_rate               :   32;
        new_timer_cindex                 :   16;
        pad                              :   48;
    }
}

header_type resp_tx_to_stage_bt_info_t {
    fields {
        log_rsq_size                     :    5;
        log_pmtu                         :    5;
        rsq_base_addr                    :   32; 
        bt_cindex                        :   16;
        end_index                        :   16;
        search_index                     :   16;
        curr_read_rsp_psn                :   24;
        read_rsp_in_progress             :    1;
        bt_in_progress                   :    1; 
        rsvd                             :   12;
    }
}

header_type resp_tx_to_stage_precheckout_info_t {
    fields {
        prefetch_cb_addr                 :   22;
        rsvd                             :  106;
    }
}

header_type resp_tx_prefetch_info_t {
    fields {
        pt_base_addr                     :   32;
        rq_pindex                        :   16;
        log_rq_page_size                 :    5;
        log_wqe_size                     :    5;
        log_num_wqes                     :    5;
        cmd_eop                          :    1;
        prefetch_cb_or_base_addr         :   32;
        prefetch_buf_index               :   16;
        check_in                         :    1;
        rsvd                             :   47;
    }
}

header_type resp_tx_bt_info_t {
    fields {
        read_or_atomic                   :    1;
        rsvd                             :    7;
        psn                              :   24;
        va                               :   64;
        r_key                            :   32;
        len                              :   32;
    }
}

header_type resp_tx_rqcb0_bt_write_back_info_t {
    fields {
        curr_read_rsp_psn                :   24;
        read_rsp_in_progress             :    1;
        bt_in_progress                   :    1;
        update_read_rsp_in_progress      :    1; 
        update_rsq_cindex                :    1;
        update_bt_cindex                 :    1;
        update_bt_in_progress            :    1;
        update_bt_rsq_cindex             :    1;
        update_drain_done                :    1;
        rsq_cindex                       :   16; 
        bt_cindex                        :   16;
        bt_rsq_cindex                    :   16;
        pad                              :   80;
    }
}

header_type resp_tx_s6_info_t {
    fields {
        rsvd                             :  128;
    }
}

header_type resp_tx_rqcb_to_rqcb1_info_t {
    fields {
        rsqwqe_addr                      :   64;
        log_pmtu                         :    5;
        serv_type                        :    3;
        timer_event_process              :    1;
        curr_read_rsp_psn                :   24;
        read_rsp_in_progress             :    1;
        pad                              :   62;
    }
}

header_type resp_tx_rsqwqe_to_rkey_info_t {
    fields {
        transfer_va                      :   64;
        header_template_addr             :   32;
        curr_read_rsp_psn                :   24;
        log_pmtu                         :    5;
        key_id                           :    1;
        send_aeth                        :    1;
        last_or_only                     :    1;
        transfer_bytes                   :   16;
        header_template_size             :    8;
        skip_rkey                        :    1;
        pad                              :    7;
    }
}

header_type resp_tx_rqcb_to_cnp_info_t {
    fields {
        new_c_index                      :   16;
        pad                              :  144;
    }
}

header_type resp_tx_rqcb2_to_rsqwqe_info_t {
    fields {
        curr_read_rsp_psn                :   24;
        log_pmtu                         :    5;
        serv_type                        :    3;
        header_template_addr             :   32;
        header_template_size             :    8;
        pad                              :   88;
    }
}

header_type resp_tx_to_stage_rqpt_info_t {
    fields {
        prefetch_base_addr               :   32;
        page_offset                      :   16;
        prefetch_pindex_pre              :   16;
        prefetch_pindex_post             :   16;
        transfer_num_wqes                :   16;
        page_seg_offset                  :    3;
        log_wqe_size                     :    5;
        cmd_eop                          :    1;
        invoke_stats                     :    1;
        pad                              :   22;
    }
}

header_type resp_tx_s0_info_t {
    fields {
        rsvd                             :  128;
    }
}

header_type resp_tx_s7_info_t {
    fields {
        rsvd                             :  128;
    }
}

header_type resp_tx_to_stage_stats_info_t {
    fields {
        pyld_bytes                       :   16;
        last_psn                         :   24;   
        last_syndrome                    :    8;
        last_msn                         :   24;
        lif_error_id_vld                 :    1;
        lif_error_id                     :    4;
        flush_rq                         :    1;
        incr_prefetch_cnt                :    1;
        prefetch_only                    :    1;
        qp_err_disabled                  :    1;
        qp_err_dis_rsvd_rkey_err         :    1;
        qp_err_dis_rkey_state_err        :    1;
        qp_err_dis_rkey_pd_mismatch      :    1;
        qp_err_dis_rkey_acc_ctrl_err     :    1;
        qp_err_dis_rkey_va_err           :    1;
        qp_err_dis_type2a_mw_qp_mismatch :    1;
        qp_err_dis_resp_rx               :    1;
        dcqcn_rate                       :    1;
        dcqcn_timer                      :    1;
        rp_num_additive_increase         :    1;
        rp_num_fast_recovery             :    1;
        rp_num_hyper_increase            :    1;
        rp_num_max_rate_reached          :    1;
        rp_num_byte_threshold_db         :    1;
        rp_num_alpha_timer_expiry        :    1;
        rp_num_timer_T_expiry            :    1;
        pad                              :   31;
    }
}

header_type resp_tx_stats_info_t {
    fields {
        rsvd                             :  160;
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

@pragma pa_header_union ingress to_stage_1
metadata resp_tx_to_stage_dcqcn_info_t to_s1_dcqcn_info;
@pragma scratch_metadata
metadata resp_tx_to_stage_dcqcn_info_t to_s1_dcqcn_info_scr;

@pragma pa_header_union ingress to_stage_1
metadata resp_tx_to_stage_bt_info_t to_s1_bt_info;
@pragma scratch_metadata
metadata resp_tx_to_stage_bt_info_t to_s1_bt_info_scr;

@pragma pa_header_union ingress to_stage_1
metadata resp_tx_to_stage_precheckout_info_t to_s1_precheckout_info;
@pragma scratch_metadata
metadata resp_tx_to_stage_precheckout_info_t to_s1_precheckout_info_scr;

@pragma pa_header_union ingress to_stage_2
metadata resp_tx_to_stage_dcqcn_info_t to_s2_dcqcn_info;
@pragma scratch_metadata
metadata resp_tx_to_stage_dcqcn_info_t to_s2_dcqcn_info_scr;

@pragma pa_header_union ingress to_stage_2
metadata resp_tx_to_stage_bt_info_t to_s2_bt_info;
@pragma scratch_metadata
metadata resp_tx_to_stage_bt_info_t to_s2_bt_info_scr;

@pragma pa_header_union ingress to_stage_3
metadata resp_tx_to_stage_dcqcn_info_t to_s3_dcqcn_info;
@pragma scratch_metadata
metadata resp_tx_to_stage_dcqcn_info_t to_s3_dcqcn_info_scr;

@pragma pa_header_union ingress to_stage_3
metadata resp_tx_to_stage_bt_info_t to_s3_bt_info;
@pragma scratch_metadata
metadata resp_tx_to_stage_bt_info_t to_s3_bt_info_scr;

@pragma pa_header_union ingress to_stage_4
metadata resp_tx_to_stage_dcqcn_info_t to_s4_dcqcn_info;
@pragma scratch_metadata
metadata resp_tx_to_stage_dcqcn_info_t to_s4_dcqcn_info_scr;

@pragma pa_header_union ingress to_stage_4
metadata resp_tx_to_stage_dcqcn_rate_timer_t to_s4_dcqcn_rate_timer_info;
@pragma scratch_metadata
metadata resp_tx_to_stage_dcqcn_rate_timer_t to_s4_dcqcn_rate_timer_info_scr;

@pragma pa_header_union ingress to_stage_5
metadata resp_tx_to_stage_rqcb1_wb_info_t to_s5_rqcb1_wb_info;
@pragma scratch_metadata
metadata resp_tx_to_stage_rqcb1_wb_info_t to_s5_rqcb1_wb_info_scr;

@pragma pa_header_union ingress to_stage_6
metadata resp_tx_to_stage_rqpt_info_t to_s6_rqpt_info;
@pragma scratch_metadata
metadata resp_tx_to_stage_rqpt_info_t to_s6_rqpt_info_scr;

@pragma pa_header_union ingress to_stage_7
metadata resp_tx_to_stage_stats_info_t to_s7_stats_info;
@pragma scratch_metadata
metadata resp_tx_to_stage_stats_info_t to_s7_stats_info_scr;

/**** stage to stage header unions ****/

@pragma pa_header_union ingress common_t0_s2s
metadata resp_tx_rqcb_to_rqcb2_info_t t0_s2s_rqcb_to_rqcb2_info;
@pragma scratch_metadata
metadata resp_tx_rqcb_to_rqcb2_info_t t0_s2s_rqcb_to_rqcb2_info_scr;

@pragma pa_header_union ingress common_t0_s2s
metadata resp_tx_rqcb_to_ack_info_t t0_s2s_rqcb_to_ack_info;
@pragma scratch_metadata
metadata resp_tx_rqcb_to_ack_info_t t0_s2s_rqcb_to_ack_info_scr;

@pragma pa_header_union ingress common_t0_s2s
metadata resp_tx_rsq_backtrack_adjust_info_t t0_s2s_rsq_backtrack_adjust_info;
@pragma scratch_metadata
metadata resp_tx_rsq_backtrack_adjust_info_t t0_s2s_rsq_backtrack_adjust_info_scr;

@pragma pa_header_union ingress common_t0_s2s
metadata resp_tx_rqcb2_to_rsqwqe_info_t t0_s2s_rqcb2_to_rsqwqe_info;
@pragma scratch_metadata
metadata resp_tx_rqcb2_to_rsqwqe_info_t t0_s2s_rqcb2_to_rsqwqe_info_scr;

@pragma pa_header_union ingress common_t0_s2s
metadata resp_tx_rsqwqe_to_rkey_info_t t0_s2s_rsqwqe_to_rkey_info;
@pragma scratch_metadata
metadata resp_tx_rsqwqe_to_rkey_info_t t0_s2s_rsqwqe_to_rkey_info_scr;

@pragma pa_header_union ingress common_t0_s2s
metadata resp_tx_rkey_to_ptseg_info_t t0_s2s_rkey_to_ptseg_info;
@pragma scratch_metadata
metadata resp_tx_rkey_to_ptseg_info_t t0_s2s_rkey_to_ptseg_info_scr;

@pragma pa_header_union ingress common_t0_s2s
metadata resp_tx_cfg_to_dcqcn_info_t t0_s2s_cfg_to_dcqcn_info;
@pragma scratch_metadata
metadata resp_tx_cfg_to_dcqcn_info_t t0_s2s_cfg_to_dcqcn_info_scr;

@pragma pa_header_union ingress common_t1_s2s
metadata resp_tx_rqcb0_write_back_info_t t1_s2s_rqcb0_write_back_info;
@pragma scratch_metadata
metadata resp_tx_rqcb0_write_back_info_t t1_s2s_rqcb0_write_back_info_scr;

@pragma pa_header_union ingress common_t1_s2s
metadata resp_tx_rqcb0_bt_write_back_info_t t1_s2s_rqcb0_bt_write_back_info;
@pragma scratch_metadata
metadata resp_tx_rqcb0_bt_write_back_info_t t1_s2s_rqcb0_bt_write_back_info_scr;

@pragma pa_header_union ingress common_t1_s2s
metadata resp_tx_rqcb_to_dcqcn_cfg_info_t t1_s2s_dcqcn_config_info;
@pragma scratch_metadata
metadata resp_tx_rqcb_to_dcqcn_cfg_info_t t1_s2s_dcqcn_config_info_scr;

@pragma pa_header_union ingress common_t0_s2s
metadata resp_tx_bt_info_t t0_s2s_bt_info;
@pragma scratch_metadata
metadata resp_tx_bt_info_t t0_s2s_bt_info_scr;

@pragma pa_header_union ingress common_t2_s2s
metadata resp_tx_prefetch_info_t t2_s2s_prefetch_info;
@pragma scratch_metadata
metadata resp_tx_prefetch_info_t t2_s2s_prefetch_info_scr;

@pragma pa_header_union ingress common_t3_s2s
metadata resp_tx_rsqrkey_to_rkey_cookie_info_t t3_s2s_rsqrkey_to_rkey_cookie_info;
@pragma scratch_metadata
metadata resp_tx_rsqrkey_to_rkey_cookie_info_t t3_s2s_rsqrkey_to_rkey_cookie_info_scr;

@pragma pa_header_union ingress common_t3_s2s
metadata resp_tx_stats_info_t t3_s2s_stats_info;
@pragma scratch_metadata
metadata resp_tx_stats_info_t t3_s2s_stats_info_scr;


/*
 * Stage 0 table 0 action
 */
action rdma_stage0_table_action() {

    // k + i for stage 0
}

/*
 * Stage 0 table 0 recirc action
 */
action rdma_stage0_recirc_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_txdma_intr_scratch.qid, p4_txdma_intr.qid);
    modify_field(p4_txdma_intr_scratch.qtype, p4_txdma_intr.qtype);
    modify_field(p4_txdma_intr_scratch.qstate_addr, p4_txdma_intr.qstate_addr);

    // recirc header bits
    modify_field(rdma_recirc_scr.recirc_reason, rdma_recirc.recirc_reason);
}

action resp_tx_ack_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t0_s2s_rqcb_to_ack_info_scr.header_template_addr, t0_s2s_rqcb_to_ack_info.header_template_addr);
    modify_field(t0_s2s_rqcb_to_ack_info_scr.pad, t0_s2s_rqcb_to_ack_info.pad);

}
#if 0
action resp_tx_dcqcn_enforce_process_s2 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s2_dcqcn_info_scr.dcqcn_cb_addr, to_s2_dcqcn_info.dcqcn_cb_addr);
    modify_field(to_s2_dcqcn_info_scr.congestion_mgmt_type, to_s2_dcqcn_info.congestion_mgmt_type);
    modify_field(to_s2_dcqcn_info_scr.packet_len, to_s2_dcqcn_info.packet_len);
    modify_field(to_s2_dcqcn_info_scr.rsvd, to_s2_dcqcn_info.rsvd);

    // stage to stage
    modify_field(t1_s2s_rqcb0_write_back_info_scr.curr_read_rsp_psn, t1_s2s_rqcb0_write_back_info.curr_read_rsp_psn);
    modify_field(t1_s2s_rqcb0_write_back_info_scr.read_rsp_in_progress, t1_s2s_rqcb0_write_back_info.read_rsp_in_progress);
    modify_field(t1_s2s_rqcb0_write_back_info_scr.rate_enforce_failed, t1_s2s_rqcb0_write_back_info.rate_enforce_failed);
    modify_field(t1_s2s_rqcb0_write_back_info_scr.pad, t1_s2s_rqcb0_write_back_info.pad);

}
#endif
action resp_tx_dcqcn_enforce_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_dcqcn_info_scr.dcqcn_cb_addr, to_s4_dcqcn_info.dcqcn_cb_addr);
    modify_field(to_s4_dcqcn_info_scr.congestion_mgmt_type, to_s4_dcqcn_info.congestion_mgmt_type);
    modify_field(to_s4_dcqcn_info_scr.packet_len, to_s4_dcqcn_info.packet_len);
    modify_field(to_s4_dcqcn_info_scr.resp_rl_failure, to_s4_dcqcn_info.resp_rl_failure);
    modify_field(to_s4_dcqcn_info_scr.rsvd, to_s4_dcqcn_info.rsvd);

    // stage to stage
    modify_field(t1_s2s_rqcb0_write_back_info_scr.curr_read_rsp_psn, t1_s2s_rqcb0_write_back_info.curr_read_rsp_psn);
    modify_field(t1_s2s_rqcb0_write_back_info_scr.read_rsp_in_progress, t1_s2s_rqcb0_write_back_info.read_rsp_in_progress);
    modify_field(t1_s2s_rqcb0_write_back_info_scr.rate_enforce_failed, t1_s2s_rqcb0_write_back_info.rate_enforce_failed);
    modify_field(t1_s2s_rqcb0_write_back_info_scr.pad, t1_s2s_rqcb0_write_back_info.pad);

}

action resp_tx_dcqcn_rate_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_dcqcn_rate_timer_info.min_qp_rate, to_s4_dcqcn_rate_timer_info.min_qp_rate);
    modify_field(to_s4_dcqcn_rate_timer_info.min_qp_target_rate, to_s4_dcqcn_rate_timer_info.min_qp_target_rate);
    modify_field(to_s4_dcqcn_rate_timer_info.new_timer_cindex, to_s4_dcqcn_rate_timer_info.new_timer_cindex);
    modify_field(to_s4_dcqcn_rate_timer_info.pad, to_s4_dcqcn_rate_timer_info.pad);

    // stage to stage
    modify_field(t0_s2s_cfg_to_dcqcn_info_scr.alpha_timer_interval, t0_s2s_cfg_to_dcqcn_info.alpha_timer_interval);
    modify_field(t0_s2s_cfg_to_dcqcn_info_scr.timer_exp_thr, t0_s2s_cfg_to_dcqcn_info.timer_exp_thr);
    modify_field(t0_s2s_cfg_to_dcqcn_info_scr.g_val, t0_s2s_cfg_to_dcqcn_info.g_val);
    modify_field(t0_s2s_cfg_to_dcqcn_info_scr.ai_rate, t0_s2s_cfg_to_dcqcn_info.ai_rate);
    modify_field(t0_s2s_cfg_to_dcqcn_info_scr.hai_rate, t0_s2s_cfg_to_dcqcn_info.hai_rate);
    modify_field(t0_s2s_cfg_to_dcqcn_info_scr.threshold, t0_s2s_cfg_to_dcqcn_info.threshold);
    modify_field(t0_s2s_cfg_to_dcqcn_info_scr.byte_reset, t0_s2s_cfg_to_dcqcn_info.byte_reset);
    modify_field(t0_s2s_cfg_to_dcqcn_info_scr.clamp_tgt_rate, t0_s2s_cfg_to_dcqcn_info.clamp_tgt_rate);
    modify_field(t0_s2s_cfg_to_dcqcn_info_scr.pad, t0_s2s_cfg_to_dcqcn_info.pad);

}
action resp_tx_dcqcn_timer_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t0_s2s_cfg_to_dcqcn_info_scr.alpha_timer_interval, t0_s2s_cfg_to_dcqcn_info.alpha_timer_interval);
    modify_field(t0_s2s_cfg_to_dcqcn_info_scr.timer_exp_thr, t0_s2s_cfg_to_dcqcn_info.timer_exp_thr);
    modify_field(t0_s2s_cfg_to_dcqcn_info_scr.g_val, t0_s2s_cfg_to_dcqcn_info.g_val);

}
action resp_tx_dcqcn_config_load_process() {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t1_s2s_dcqcn_config_info_scr.dcqcn_config_base, t1_s2s_dcqcn_config_info.dcqcn_config_base);
    modify_field(t1_s2s_dcqcn_config_info_scr.dcqcn_config_id, t1_s2s_dcqcn_config_info.dcqcn_config_id);
    modify_field(t1_s2s_dcqcn_config_info_scr.dcqcn_rate_timer_toggle, t1_s2s_dcqcn_config_info.dcqcn_rate_timer_toggle);
    modify_field(t1_s2s_dcqcn_config_info_scr.pad, t1_s2s_dcqcn_config_info.pad);

}
action resp_tx_rqcb0_write_back_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s5_rqcb1_wb_info_scr.new_c_index, to_s5_rqcb1_wb_info.new_c_index);
    modify_field(to_s5_rqcb1_wb_info_scr.ack_nak_process, to_s5_rqcb1_wb_info.ack_nak_process);
    modify_field(to_s5_rqcb1_wb_info_scr.flush_rq, to_s5_rqcb1_wb_info.flush_rq);
    modify_field(to_s5_rqcb1_wb_info_scr.rsvd, to_s5_rqcb1_wb_info.rsvd);

    // stage to stage
    modify_field(t1_s2s_rqcb0_write_back_info_scr.curr_read_rsp_psn, t1_s2s_rqcb0_write_back_info.curr_read_rsp_psn);
    modify_field(t1_s2s_rqcb0_write_back_info_scr.read_rsp_in_progress, t1_s2s_rqcb0_write_back_info.read_rsp_in_progress);
    modify_field(t1_s2s_rqcb0_write_back_info_scr.rate_enforce_failed, t1_s2s_rqcb0_write_back_info.rate_enforce_failed);
    modify_field(t1_s2s_rqcb0_write_back_info_scr.pad, t1_s2s_rqcb0_write_back_info.pad);

}
action resp_tx_rqcb0_bt_write_back_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s3_bt_info_scr.log_rsq_size, to_s3_bt_info.log_rsq_size);
    modify_field(to_s3_bt_info_scr.log_pmtu, to_s3_bt_info.log_pmtu);
    modify_field(to_s3_bt_info_scr.rsq_base_addr, to_s3_bt_info.rsq_base_addr);
    modify_field(to_s3_bt_info_scr.bt_cindex, to_s3_bt_info.bt_cindex);
    modify_field(to_s3_bt_info_scr.end_index, to_s3_bt_info.end_index);
    modify_field(to_s3_bt_info_scr.search_index, to_s3_bt_info.search_index);
    modify_field(to_s3_bt_info_scr.curr_read_rsp_psn, to_s3_bt_info.curr_read_rsp_psn);
    modify_field(to_s3_bt_info_scr.read_rsp_in_progress, to_s3_bt_info.read_rsp_in_progress);
    modify_field(to_s3_bt_info_scr.bt_in_progress, to_s3_bt_info.bt_in_progress);
    modify_field(to_s3_bt_info_scr.rsvd, to_s3_bt_info.rsvd);
    
    // stage to stage
    modify_field(t1_s2s_rqcb0_bt_write_back_info_scr.curr_read_rsp_psn, t1_s2s_rqcb0_bt_write_back_info.curr_read_rsp_psn);
    modify_field(t1_s2s_rqcb0_bt_write_back_info_scr.read_rsp_in_progress, t1_s2s_rqcb0_bt_write_back_info.read_rsp_in_progress);
    modify_field(t1_s2s_rqcb0_bt_write_back_info_scr.bt_in_progress, t1_s2s_rqcb0_bt_write_back_info.bt_in_progress);
    modify_field(t1_s2s_rqcb0_bt_write_back_info_scr.update_read_rsp_in_progress, t1_s2s_rqcb0_bt_write_back_info.update_read_rsp_in_progress);
    modify_field(t1_s2s_rqcb0_bt_write_back_info_scr.update_rsq_cindex, t1_s2s_rqcb0_bt_write_back_info.update_rsq_cindex);
    modify_field(t1_s2s_rqcb0_bt_write_back_info_scr.update_bt_cindex, t1_s2s_rqcb0_bt_write_back_info.update_bt_cindex);
    modify_field(t1_s2s_rqcb0_bt_write_back_info_scr.update_bt_in_progress, t1_s2s_rqcb0_bt_write_back_info.update_bt_in_progress);
    modify_field(t1_s2s_rqcb0_bt_write_back_info_scr.update_bt_rsq_cindex, t1_s2s_rqcb0_bt_write_back_info.update_bt_rsq_cindex);
    modify_field(t1_s2s_rqcb0_bt_write_back_info_scr.update_drain_done, t1_s2s_rqcb0_bt_write_back_info.update_drain_done);
    modify_field(t1_s2s_rqcb0_bt_write_back_info_scr.rsq_cindex, t1_s2s_rqcb0_bt_write_back_info.rsq_cindex);
    modify_field(t1_s2s_rqcb0_bt_write_back_info_scr.bt_cindex, t1_s2s_rqcb0_bt_write_back_info.bt_cindex);
    modify_field(t1_s2s_rqcb0_bt_write_back_info_scr.bt_rsq_cindex, t1_s2s_rqcb0_bt_write_back_info.bt_rsq_cindex);
    modify_field(t1_s2s_rqcb0_bt_write_back_info_scr.pad, t1_s2s_rqcb0_bt_write_back_info.pad);
}
action resp_tx_rqcb2_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t0_s2s_rqcb_to_rqcb2_info_scr.rsqwqe_addr, t0_s2s_rqcb_to_rqcb2_info.rsqwqe_addr);
    modify_field(t0_s2s_rqcb_to_rqcb2_info_scr.curr_read_rsp_psn, t0_s2s_rqcb_to_rqcb2_info.curr_read_rsp_psn);
    modify_field(t0_s2s_rqcb_to_rqcb2_info_scr.log_pmtu, t0_s2s_rqcb_to_rqcb2_info.log_pmtu);
    modify_field(t0_s2s_rqcb_to_rqcb2_info_scr.serv_type, t0_s2s_rqcb_to_rqcb2_info.serv_type);
    modify_field(t0_s2s_rqcb_to_rqcb2_info_scr.header_template_addr, t0_s2s_rqcb_to_rqcb2_info.header_template_addr);
    modify_field(t0_s2s_rqcb_to_rqcb2_info_scr.header_template_size, t0_s2s_rqcb_to_rqcb2_info.header_template_size);
    modify_field(t0_s2s_rqcb_to_rqcb2_info_scr.pad, t0_s2s_rqcb_to_rqcb2_info.pad);

}
action resp_tx_rqcb2_bt_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_bt_info_scr.log_rsq_size, to_s1_bt_info.log_rsq_size);
    modify_field(to_s1_bt_info_scr.log_pmtu, to_s1_bt_info.log_pmtu);
    modify_field(to_s1_bt_info_scr.rsq_base_addr, to_s1_bt_info.rsq_base_addr);
    modify_field(to_s1_bt_info_scr.bt_cindex, to_s1_bt_info.bt_cindex);
    modify_field(to_s1_bt_info_scr.end_index, to_s1_bt_info.end_index);
    modify_field(to_s1_bt_info_scr.search_index, to_s1_bt_info.search_index);
    modify_field(to_s1_bt_info_scr.curr_read_rsp_psn, to_s1_bt_info.curr_read_rsp_psn);
    modify_field(to_s1_bt_info_scr.read_rsp_in_progress, to_s1_bt_info.read_rsp_in_progress);
    modify_field(to_s1_bt_info_scr.bt_in_progress, to_s1_bt_info.bt_in_progress);
    modify_field(to_s1_bt_info_scr.rsvd, to_s1_bt_info.rsvd);

    // stage to stage
}
action resp_tx_rqcb_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage

}

action resp_tx_setup_checkout_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_precheckout_info_scr.prefetch_cb_addr, to_s1_precheckout_info.prefetch_cb_addr);
    modify_field(to_s1_precheckout_info_scr.rsvd, to_s1_precheckout_info.rsvd);

    // stage to stage
    modify_field(t2_s2s_prefetch_info_scr.pt_base_addr, t2_s2s_prefetch_info.pt_base_addr);
    modify_field(t2_s2s_prefetch_info_scr.rq_pindex, t2_s2s_prefetch_info.rq_pindex);
    modify_field(t2_s2s_prefetch_info_scr.log_rq_page_size, t2_s2s_prefetch_info.log_rq_page_size);
    modify_field(t2_s2s_prefetch_info_scr.log_wqe_size, t2_s2s_prefetch_info.log_wqe_size);
    modify_field(t2_s2s_prefetch_info_scr.log_num_wqes, t2_s2s_prefetch_info.log_num_wqes);
    modify_field(t2_s2s_prefetch_info_scr.cmd_eop, t2_s2s_prefetch_info.cmd_eop);
    modify_field(t2_s2s_prefetch_info_scr.prefetch_cb_or_base_addr, t2_s2s_prefetch_info.prefetch_cb_or_base_addr);
    modify_field(t2_s2s_prefetch_info_scr.prefetch_buf_index, t2_s2s_prefetch_info.prefetch_buf_index);
    modify_field(t2_s2s_prefetch_info_scr.check_in, t2_s2s_prefetch_info.check_in);
    modify_field(t2_s2s_prefetch_info_scr.rsvd, t2_s2s_prefetch_info.rsvd);
}

action resp_tx_pre_checkout_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t2_s2s_prefetch_info_scr.pt_base_addr, t2_s2s_prefetch_info.pt_base_addr);
    modify_field(t2_s2s_prefetch_info_scr.rq_pindex, t2_s2s_prefetch_info.rq_pindex);
    modify_field(t2_s2s_prefetch_info_scr.log_rq_page_size, t2_s2s_prefetch_info.log_rq_page_size);
    modify_field(t2_s2s_prefetch_info_scr.log_wqe_size, t2_s2s_prefetch_info.log_wqe_size);
    modify_field(t2_s2s_prefetch_info_scr.log_num_wqes, t2_s2s_prefetch_info.log_num_wqes);
    modify_field(t2_s2s_prefetch_info_scr.cmd_eop, t2_s2s_prefetch_info.cmd_eop);
    modify_field(t2_s2s_prefetch_info_scr.prefetch_cb_or_base_addr, t2_s2s_prefetch_info.prefetch_cb_or_base_addr);
    modify_field(t2_s2s_prefetch_info_scr.prefetch_buf_index, t2_s2s_prefetch_info.prefetch_buf_index);
    modify_field(t2_s2s_prefetch_info_scr.check_in, t2_s2s_prefetch_info.check_in);
    modify_field(t2_s2s_prefetch_info_scr.rsvd, t2_s2s_prefetch_info.rsvd);
}

action resp_tx_checkout_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t2_s2s_prefetch_info_scr.pt_base_addr, t2_s2s_prefetch_info.pt_base_addr);
    modify_field(t2_s2s_prefetch_info_scr.rq_pindex, t2_s2s_prefetch_info.rq_pindex);
    modify_field(t2_s2s_prefetch_info_scr.log_rq_page_size, t2_s2s_prefetch_info.log_rq_page_size);
    modify_field(t2_s2s_prefetch_info_scr.log_wqe_size, t2_s2s_prefetch_info.log_wqe_size);
    modify_field(t2_s2s_prefetch_info_scr.log_num_wqes, t2_s2s_prefetch_info.log_num_wqes);
    modify_field(t2_s2s_prefetch_info_scr.cmd_eop, t2_s2s_prefetch_info.cmd_eop);
    modify_field(t2_s2s_prefetch_info_scr.prefetch_cb_or_base_addr, t2_s2s_prefetch_info.prefetch_cb_or_base_addr);
    modify_field(t2_s2s_prefetch_info_scr.prefetch_buf_index, t2_s2s_prefetch_info.prefetch_buf_index);
    modify_field(t2_s2s_prefetch_info_scr.check_in, t2_s2s_prefetch_info.check_in);
    modify_field(t2_s2s_prefetch_info_scr.rsvd, t2_s2s_prefetch_info.rsvd);
}

action resp_tx_rqprefetch_mpu_only_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
}

action resp_tx_rqprefetch_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t2_s2s_prefetch_info_scr.pt_base_addr, t2_s2s_prefetch_info.pt_base_addr);
    modify_field(t2_s2s_prefetch_info_scr.rq_pindex, t2_s2s_prefetch_info.rq_pindex);
    modify_field(t2_s2s_prefetch_info_scr.log_rq_page_size, t2_s2s_prefetch_info.log_rq_page_size);
    modify_field(t2_s2s_prefetch_info_scr.log_wqe_size, t2_s2s_prefetch_info.log_wqe_size);
    modify_field(t2_s2s_prefetch_info_scr.log_num_wqes, t2_s2s_prefetch_info.log_num_wqes);
    modify_field(t2_s2s_prefetch_info_scr.cmd_eop, t2_s2s_prefetch_info.cmd_eop);
    modify_field(t2_s2s_prefetch_info_scr.prefetch_cb_or_base_addr, t2_s2s_prefetch_info.prefetch_cb_or_base_addr);
    modify_field(t2_s2s_prefetch_info_scr.prefetch_buf_index, t2_s2s_prefetch_info.prefetch_buf_index);
    modify_field(t2_s2s_prefetch_info_scr.check_in, t2_s2s_prefetch_info.check_in);
    modify_field(t2_s2s_prefetch_info_scr.rsvd, t2_s2s_prefetch_info.rsvd);
}

action resp_tx_rqpt_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s6_rqpt_info_scr.prefetch_base_addr, to_s6_rqpt_info.prefetch_base_addr);
    modify_field(to_s6_rqpt_info_scr.page_offset, to_s6_rqpt_info.page_offset);
    modify_field(to_s6_rqpt_info_scr.prefetch_pindex_pre, to_s6_rqpt_info.prefetch_pindex_pre);
    modify_field(to_s6_rqpt_info_scr.prefetch_pindex_post, to_s6_rqpt_info.prefetch_pindex_post);
    modify_field(to_s6_rqpt_info_scr.transfer_num_wqes, to_s6_rqpt_info.transfer_num_wqes);
    modify_field(to_s6_rqpt_info_scr.page_seg_offset, to_s6_rqpt_info.page_seg_offset);
    modify_field(to_s6_rqpt_info_scr.log_wqe_size, to_s6_rqpt_info.log_wqe_size);
    modify_field(to_s6_rqpt_info_scr.cmd_eop, to_s6_rqpt_info.cmd_eop);
    modify_field(to_s6_rqpt_info_scr.invoke_stats, to_s6_rqpt_info.invoke_stats);
    modify_field(to_s6_rqpt_info_scr.pad, to_s6_rqpt_info.pad);

    // stage to stage
}

action resp_tx_rqpt_mpu_only_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
}

action resp_tx_rsq_backtrack_adjust_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t0_s2s_rsq_backtrack_adjust_info_scr.adjust_rsq_c_index, t0_s2s_rsq_backtrack_adjust_info.adjust_rsq_c_index);
    modify_field(t0_s2s_rsq_backtrack_adjust_info_scr.rsq_bt_p_index, t0_s2s_rsq_backtrack_adjust_info.rsq_bt_p_index);
    modify_field(t0_s2s_rsq_backtrack_adjust_info_scr.pad, t0_s2s_rsq_backtrack_adjust_info.pad);

}
action resp_tx_rsqptseg_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t0_s2s_rkey_to_ptseg_info_scr.pt_seg_offset, t0_s2s_rkey_to_ptseg_info.pt_seg_offset);
    modify_field(t0_s2s_rkey_to_ptseg_info_scr.pt_seg_bytes, t0_s2s_rkey_to_ptseg_info.pt_seg_bytes);
    modify_field(t0_s2s_rkey_to_ptseg_info_scr.dma_cmd_start_index, t0_s2s_rkey_to_ptseg_info.dma_cmd_start_index);
    modify_field(t0_s2s_rkey_to_ptseg_info_scr.log_page_size, t0_s2s_rkey_to_ptseg_info.log_page_size);
    modify_field(t0_s2s_rkey_to_ptseg_info_scr.tbl_id, t0_s2s_rkey_to_ptseg_info.tbl_id);
    modify_field(t0_s2s_rkey_to_ptseg_info_scr.host_addr, t0_s2s_rkey_to_ptseg_info.host_addr);
    modify_field(t0_s2s_rkey_to_ptseg_info_scr.pad, t0_s2s_rkey_to_ptseg_info.pad);

}
action resp_tx_rsqrkey_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s3_dcqcn_info_scr.dcqcn_cb_addr, to_s3_dcqcn_info.dcqcn_cb_addr);
    modify_field(to_s3_dcqcn_info_scr.congestion_mgmt_type, to_s3_dcqcn_info.congestion_mgmt_type);
    modify_field(to_s3_dcqcn_info_scr.packet_len, to_s3_dcqcn_info.packet_len);
    modify_field(to_s3_dcqcn_info_scr.rsvd, to_s3_dcqcn_info.rsvd);
    modify_field(to_s3_dcqcn_info_scr.pd, to_s3_dcqcn_info.pd);


    // stage to stage
    modify_field(t0_s2s_rsqwqe_to_rkey_info_scr.transfer_va, t0_s2s_rsqwqe_to_rkey_info.transfer_va);
    modify_field(t0_s2s_rsqwqe_to_rkey_info_scr.header_template_addr, t0_s2s_rsqwqe_to_rkey_info.header_template_addr);
    modify_field(t0_s2s_rsqwqe_to_rkey_info_scr.curr_read_rsp_psn, t0_s2s_rsqwqe_to_rkey_info.curr_read_rsp_psn);
    modify_field(t0_s2s_rsqwqe_to_rkey_info_scr.log_pmtu, t0_s2s_rsqwqe_to_rkey_info.log_pmtu);
    modify_field(t0_s2s_rsqwqe_to_rkey_info_scr.key_id, t0_s2s_rsqwqe_to_rkey_info.key_id);
    modify_field(t0_s2s_rsqwqe_to_rkey_info_scr.send_aeth, t0_s2s_rsqwqe_to_rkey_info.send_aeth);
    modify_field(t0_s2s_rsqwqe_to_rkey_info_scr.last_or_only, t0_s2s_rsqwqe_to_rkey_info.last_or_only);
    modify_field(t0_s2s_rsqwqe_to_rkey_info_scr.transfer_bytes, t0_s2s_rsqwqe_to_rkey_info.transfer_bytes);
    modify_field(t0_s2s_rsqwqe_to_rkey_info_scr.header_template_size, t0_s2s_rsqwqe_to_rkey_info.header_template_size);
    modify_field(t0_s2s_rsqwqe_to_rkey_info_scr.skip_rkey, t0_s2s_rsqwqe_to_rkey_info.skip_rkey);
    modify_field(t0_s2s_rsqwqe_to_rkey_info_scr.pad, t0_s2s_rsqwqe_to_rkey_info.pad);

}
action resp_tx_rsqrkey_mr_cookie_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t3_s2s_rsqrkey_to_rkey_cookie_info_scr.mw_cookie, t3_s2s_rsqrkey_to_rkey_cookie_info.mw_cookie);
    modify_field(t3_s2s_rsqrkey_to_rkey_cookie_info_scr.pad, t3_s2s_rsqrkey_to_rkey_cookie_info.pad);
}

action resp_tx_rsqwqe_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t0_s2s_rqcb2_to_rsqwqe_info_scr.curr_read_rsp_psn, t0_s2s_rqcb2_to_rsqwqe_info.curr_read_rsp_psn);
    modify_field(t0_s2s_rqcb2_to_rsqwqe_info_scr.log_pmtu, t0_s2s_rqcb2_to_rsqwqe_info.log_pmtu);
    modify_field(t0_s2s_rqcb2_to_rsqwqe_info_scr.serv_type, t0_s2s_rqcb2_to_rsqwqe_info.serv_type);
    modify_field(t0_s2s_rqcb2_to_rsqwqe_info_scr.header_template_addr, t0_s2s_rqcb2_to_rsqwqe_info.header_template_addr);
    modify_field(t0_s2s_rqcb2_to_rsqwqe_info_scr.header_template_size, t0_s2s_rqcb2_to_rsqwqe_info.header_template_size);
    modify_field(t0_s2s_rqcb2_to_rsqwqe_info_scr.pad, t0_s2s_rqcb2_to_rsqwqe_info.pad);

}
action resp_tx_rsqwqe_bt_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s2_bt_info_scr.log_rsq_size, to_s2_bt_info.log_rsq_size);
    modify_field(to_s2_bt_info_scr.log_pmtu, to_s2_bt_info.log_pmtu);
    modify_field(to_s2_bt_info_scr.rsq_base_addr, to_s2_bt_info.rsq_base_addr);
    modify_field(to_s2_bt_info_scr.bt_cindex, to_s2_bt_info.bt_cindex);
    modify_field(to_s2_bt_info_scr.end_index, to_s2_bt_info.end_index);
    modify_field(to_s2_bt_info_scr.search_index, to_s2_bt_info.search_index);
    modify_field(to_s2_bt_info_scr.curr_read_rsp_psn, to_s2_bt_info.curr_read_rsp_psn);
    modify_field(to_s2_bt_info_scr.read_rsp_in_progress, to_s2_bt_info.read_rsp_in_progress);
    modify_field(to_s2_bt_info_scr.bt_in_progress, to_s2_bt_info.bt_in_progress);
    modify_field(to_s2_bt_info_scr.rsvd, to_s2_bt_info.rsvd);

    // stage to stage
    modify_field(t0_s2s_bt_info_scr.read_or_atomic, t0_s2s_bt_info.read_or_atomic);
    modify_field(t0_s2s_bt_info_scr.rsvd, t0_s2s_bt_info.rsvd);
    modify_field(t0_s2s_bt_info_scr.psn, t0_s2s_bt_info.psn);
    modify_field(t0_s2s_bt_info_scr.va, t0_s2s_bt_info.va);
    modify_field(t0_s2s_bt_info_scr.r_key, t0_s2s_bt_info.r_key);
    modify_field(t0_s2s_bt_info_scr.len, t0_s2s_bt_info.len);
}

action resp_tx_stats_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s7_stats_info_scr.pyld_bytes, to_s7_stats_info.pyld_bytes);
    modify_field(to_s7_stats_info_scr.last_psn, to_s7_stats_info.last_psn);
    modify_field(to_s7_stats_info_scr.last_syndrome, to_s7_stats_info.last_syndrome);
    modify_field(to_s7_stats_info_scr.last_msn, to_s7_stats_info.last_msn);
    modify_field(to_s7_stats_info_scr.lif_error_id_vld, to_s7_stats_info.lif_error_id_vld);
    modify_field(to_s7_stats_info_scr.lif_error_id, to_s7_stats_info.lif_error_id);
    modify_field(to_s7_stats_info_scr.flush_rq, to_s7_stats_info.flush_rq);
    modify_field(to_s7_stats_info_scr.incr_prefetch_cnt, to_s7_stats_info.incr_prefetch_cnt);
    modify_field(to_s7_stats_info_scr.prefetch_only, to_s7_stats_info.prefetch_only);
    modify_field(to_s7_stats_info_scr.qp_err_disabled, to_s7_stats_info.qp_err_disabled);
    modify_field(to_s7_stats_info_scr.qp_err_dis_rsvd_rkey_err, to_s7_stats_info.qp_err_dis_rsvd_rkey_err);
    modify_field(to_s7_stats_info_scr.qp_err_dis_rkey_state_err, to_s7_stats_info.qp_err_dis_rkey_state_err);
    modify_field(to_s7_stats_info_scr.qp_err_dis_rkey_pd_mismatch, to_s7_stats_info.qp_err_dis_rkey_pd_mismatch);
    modify_field(to_s7_stats_info_scr.qp_err_dis_rkey_acc_ctrl_err, to_s7_stats_info.qp_err_dis_rkey_acc_ctrl_err);
    modify_field(to_s7_stats_info_scr.qp_err_dis_rkey_va_err, to_s7_stats_info.qp_err_dis_rkey_va_err);
    modify_field(to_s7_stats_info_scr.qp_err_dis_type2a_mw_qp_mismatch, to_s7_stats_info.qp_err_dis_type2a_mw_qp_mismatch);
    modify_field(to_s7_stats_info_scr.qp_err_dis_resp_rx, to_s7_stats_info.qp_err_dis_resp_rx);
    modify_field(to_s7_stats_info_scr.dcqcn_rate, to_s7_stats_info.dcqcn_rate);
    modify_field(to_s7_stats_info_scr.dcqcn_timer, to_s7_stats_info.dcqcn_timer);
    modify_field(to_s7_stats_info_scr.rp_num_additive_increase, to_s7_stats_info.rp_num_additive_increase);
    modify_field(to_s7_stats_info_scr.rp_num_fast_recovery, to_s7_stats_info.rp_num_fast_recovery);
    modify_field(to_s7_stats_info_scr.rp_num_hyper_increase, to_s7_stats_info.rp_num_hyper_increase);
    modify_field(to_s7_stats_info_scr.rp_num_max_rate_reached, to_s7_stats_info.rp_num_max_rate_reached);
    modify_field(to_s7_stats_info_scr.rp_num_byte_threshold_db, to_s7_stats_info.rp_num_byte_threshold_db);
    modify_field(to_s7_stats_info_scr.rp_num_alpha_timer_expiry, to_s7_stats_info.rp_num_alpha_timer_expiry);
    modify_field(to_s7_stats_info_scr.rp_num_timer_T_expiry, to_s7_stats_info.rp_num_timer_T_expiry);
    modify_field(to_s7_stats_info_scr.pad, to_s7_stats_info.pad);

    // stage to stage
    modify_field(t3_s2s_stats_info_scr.rsvd, t3_s2s_stats_info.rsvd);

}
