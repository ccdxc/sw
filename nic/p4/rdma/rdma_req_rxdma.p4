/***********************************************************************/
/* rdma_req_rxdma.p4 */
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
#define common_p4plus_stage0_app_header_table_action_dummy18 rdma_stage0_timer_expiry_feedback_action
#define common_p4plus_stage0_app_header_table_action_dummy19 rdma_stage0_sq_drain_feedback_action

#define rx_stage0_load_rdma_params_dummy1 rdma_stage0_ext_bth_atomiceth_action
#define rx_stage0_load_rdma_params_dummy2 rdma_stage0_ext_bth_xrceth_atomiceth_action
#define rx_stage0_load_rdma_params_dummy3 rdma_stage0_ext_bth_xrceth_reth_immeth_action 
#define rx_stage0_load_rdma_params_dummy4 rdma_stage0_ext_bth_deth_immeth_action


/**** table declarations ****/

#define rx_table_s0_t0 req_rx_s0_t0
#define rx_table_s0_t1 req_rx_s0_t1
#define rx_table_s0_t2 req_rx_s0_t2
#define rx_table_s0_t3 req_rx_s0_t3

#define rx_table_s1_t0 req_rx_s1_t0
#define rx_table_s1_t1 req_rx_s1_t1
#define rx_table_s1_t2 req_rx_s1_t2
#define rx_table_s1_t3 req_rx_s1_t3

#define rx_table_s2_t0 req_rx_s2_t0
#define rx_table_s2_t1 req_rx_s2_t1
#define rx_table_s2_t2 req_rx_s2_t2
#define rx_table_s2_t3 req_rx_s2_t3

#define rx_table_s3_t0 req_rx_s3_t0
#define rx_table_s3_t1 req_rx_s3_t1
#define rx_table_s3_t2 req_rx_s3_t2
#define rx_table_s3_t3 req_rx_s3_t3

#define rx_table_s4_t0 req_rx_s4_t0
#define rx_table_s4_t1 req_rx_s4_t1
#define rx_table_s4_t2 req_rx_s4_t2
#define rx_table_s4_t3 req_rx_s4_t3

#define rx_table_s5_t0 req_rx_s5_t0
#define rx_table_s5_t1 req_rx_s5_t1
#define rx_table_s5_t2 req_rx_s5_t2
#define rx_table_s5_t3 req_rx_s5_t3

#define rx_table_s6_t0 req_rx_s6_t0
#define rx_table_s6_t1 req_rx_s6_t1
#define rx_table_s6_t2 req_rx_s6_t2
#define rx_table_s6_t3 req_rx_s6_t3

#define rx_table_s7_t0 req_rx_s7_t0
#define rx_table_s7_t1 req_rx_s7_t1
#define rx_table_s7_t2 req_rx_s7_t2
#define rx_table_s7_t3 req_rx_s7_t3


#define rx_table_s0_t0_action req_rx_sqcb1_process

#define rx_table_s1_t0_action req_rx_rrqwqe_process
#define rx_table_s1_t0_action1 req_rx_sqcb1_recirc_process
#define rx_table_s1_t3_action req_rx_dcqcn_ecn_process

#define rx_table_s2_t0_action req_rx_rrqsge_process

#define rx_table_s3_t0_action req_rx_rrqlkey_process_t0
#define rx_table_s3_t0_action1 req_rx_rrqptseg_process_t0
#define rx_table_s3_t1_action req_rx_rrqlkey_process_t1

#define rx_table_s4_t0_action req_rx_rrqptseg_process_t0
#define rx_table_s4_t1_action req_rx_rrqptseg_process_t1
#define rx_table_s4_t2_action req_rx_sqcb1_write_back_process
#define rx_table_s4_t2_action1 req_rx_timer_expiry_process
#define rx_table_s4_t2_action2 req_rx_completion_feedback_process
#define rx_table_s4_t2_action3 req_rx_sq_drain_feedback_process

#define rx_table_s5_t2_action req_rx_cqcb_process_s5
#define rx_table_s6_t2_action req_rx_cqcb_process_s6

#define rx_table_s7_t2_action req_rx_cqpt_process

#define rx_table_s7_t0_action req_rx_eqcb_process_t0
#define rx_table_s7_t1_action req_rx_eqcb_process_t1

#define rx_table_s7_t3_action req_rx_stats_process

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
    modify_field(phv_global_common_scr._feedback, phv_global_common._feedback);\
    modify_field(phv_global_common_scr._rsvd0, phv_global_common._rsvd0);\
    modify_field(phv_global_common_scr._rsvd1, phv_global_common._rsvd1);\
    modify_field(phv_global_common_scr._rsvd2, phv_global_common._rsvd2);\
    modify_field(phv_global_common_scr._rsvd3, phv_global_common._rsvd3);\
    modify_field(phv_global_common_scr._atomic_aeth, phv_global_common._atomic_aeth);\
    modify_field(phv_global_common_scr._aeth, phv_global_common._aeth);\
    modify_field(phv_global_common_scr._completion, phv_global_common._completion);\
    modify_field(phv_global_common_scr._atomic_ack, phv_global_common._atomic_ack);\
    modify_field(phv_global_common_scr._read_resp, phv_global_common._read_resp);\
    modify_field(phv_global_common_scr._only, phv_global_common._only);\
    modify_field(phv_global_common_scr._last, phv_global_common._last);\
    modify_field(phv_global_common_scr._middle, phv_global_common._middle);\
    modify_field(phv_global_common_scr._first, phv_global_common._first);\
    modify_field(phv_global_common_scr._error_disable_qp, phv_global_common._error_disable_qp);\

/**** header definitions ****/

header_type req_rx_cqcb_to_pt_info_t {
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

header_type req_rx_rrqwqe_to_cq_info_t {
    fields {
        cq_id                            :   24;
        cqe_type                         :    3;
        pad                              :  132;
    }
}

header_type req_rx_sqcb1_to_rrqwqe_info_t {
    fields {
        cur_sge_offset                   :   32;
        msg_psn                          :   24;
        cur_sge_id                       :    8;
        ssn                              :   24;
        msn                              :   24;
        rrq_in_progress                  :    1;
        rrq_empty                        :    1;
        timer_active                     :    1;
        dma_cmd_start_index              :    4;
        rrq_cindex                       :    8;
        tx_psn                           :   24;
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
        _feedback                        :    1;
        _rsvd0                           :    1;
        _rsvd1                           :    1;
        _rsvd2                           :    1;
        _rsvd3                           :    1;
        _rsvd4                           :    1;
        _atomic_aeth                     :    1;
        _aeth                            :    1;
        _completion                      :    1;
        _atomic_ack                      :    1;
        _read_resp                       :    1;
        _only                            :    1;
        _last                            :    1;
        _middle                          :    1;
        _first                           :    1;
        _error_disable_qp                :    1;
    }
}

header_type req_rx_to_stage_rrqwqe_info_t {
    fields {
        aeth_msn                         :   24;
        bth_psn                          :   24;
        aeth_syndrome                    :    8;
        remaining_payload_bytes          :   14;
        sge_opt                          :    1;
        priv_oper_enable                 :    1;
        log_pmtu                         :    5;
        log_page_size                    :    5;
    }
}

header_type req_rx_sqcb1_to_sge_recirc_info_t {
    fields {
        rrq_in_progress                  :  1;
        log_page_size                    :  5;
    }
}

header_type req_rx_to_stage_rrqsge_info_t {
    fields {
        msg_psn                          :   24;
        log_pmtu                         :    5;
        priv_oper_enable                 :    1;
    }
}

header_type req_rx_to_stage_rrqlkey_info_t {
     fields {
         pd                              : 32;
     }
}

header_type req_rx_to_stage_cq_info_t {
    fields {
        cqcb_base_addr_hi                :   24;
        log_num_cq_entries               :    4;
        bth_se                           :    1;
        async_error_event                :    1;
        async_event                      :    1;
        state                            :    3;
    }
}

header_type req_rx_to_stage_sqcb1_wb_info_t {
    fields {
        remaining_payload_bytes          :   14;
        my_token_id                      :    8;
        error_disable_qp                 :    1;
        error_drop_phv                   :    1;
        sge_opt                          :    1;
        rrq_spec_cindex                  :   16;
    }
}

header_type req_rx_to_stage_stats_info_t {
    fields {
        pyld_bytes                       :   16;
        lif_cqe_error_id_vld             :    1;
        lif_error_id_vld                 :    1;
        lif_error_id                     :    4;
        qp_err_disabled                  :    1;
        qp_err_dis_rrqlkey_pd_mismatch   :    1;
        qp_err_dis_rrqlkey_inv_state     :    1;
        qp_err_dis_rrqlkey_rsvd_lkey     :    1;
        qp_err_dis_rrqlkey_acc_no_wr_perm:    1;
        qp_err_dis_rrqlkey_acc_len_lower :    1;
        qp_err_dis_rrqlkey_acc_len_higher:    1;
        qp_err_dis_rrqsge_insuff_sges    :    1;
        qp_err_dis_rrqsge_insuff_sge_len :    1;
        qp_err_dis_rrqsge_insuff_dma_cmds:    1;
        qp_err_dis_rrqwqe_remote_inv_req_err_rcvd :    1;
        qp_err_dis_rrqwqe_remote_acc_err_rcvd     :    1;
        qp_err_dis_rrqwqe_remote_oper_err_rcvd    :    1;
        qp_err_dis_table_error           :    1;
        qp_err_dis_phv_intrinsic_error   :    1;
        qp_err_dis_table_resp_error      :    1;
        qp_err_dis_rsvd                  :    16;
        max_recirc_cnt_err               :    1;
        recirc_reason                    :    4;
        recirc_bth_opcode                :    8;
        recirc_bth_psn                   :   24; 
        pad                              :   37;
    }
}

header_type req_rx_to_stage_recirc_info_t {
    fields {
        rrqwqe_sge_list_addr             : 64;
        cur_sge_offset                   : 32;
        cur_sge_id                       : 8;
        num_valid_sges                   : 8;
        remaining_payload_bytes          : 14;
        dma_cmd_eop                      : 1;
        sge_opt                          : 1;
    }
}

header_type req_rx_rrqwqe_to_sge_info_t {
    fields {
        remaining_payload_bytes          :   14;
        pad1                             :    1;
        post_cq                          :    1;
        cur_sge_offset                   :   32;
        cur_sge_id                       :    8;
        num_valid_sges                   :    8;
        rrq_in_progress                  :    1;
        is_atomic                        :    1;
        dma_cmd_eop                      :    1;
        dma_cmd_start_index              :    4;
        end_of_page                      :    1;
    }
}

header_type req_rx_ecn_info_t {
    fields {
        p_key                            :   16;
        pad                              :  144;
    }
}

header_type req_rx_sqcb0_to_sqcb1_info_t {
    fields {
        remaining_payload_bytes          :   14;
        pad1                             :    2;
        rrq_cindex                       :    8;
        rrq_empty                        :    1;
        need_credits                     :    1;
        dma_cmd_start_index              :    4;
        ecn_set                          :    1;
        p_key                            :   16;
        pad                              :   103;
    }
}

header_type req_rx_rrqlkey_to_ptseg_info_t {
    fields {
        pt_offset                        :   32;
        pt_bytes                         :   16;
        log_page_size                    :    5;
        dma_cmd_start_index              :    4;
        is_atomic                        :    1;
        dma_cmd_eop                      :    1;
        sge_index                        :    8;
        pad                              :   93;
    }
}

header_type req_rx_cqcb_to_eq_info_t {
    fields {
        async_eq                         :  1;
        cmd_eop                          :  1;
        donot_reset_tbl_vld              :  1;
        pad                              :  157;
    }
}

header_type req_rx_sqcb1_write_back_info_t {
    fields {
        cur_sge_offset                   :   32;
        msg_psn                          :   24;
        cur_sge_id                       :    8;
        rrq_in_progress                  :    1;
        incr_nxt_to_go_token_id          :    1;
        post_bktrack                     :    1;
        post_cq                          :    1;
        last_pkt                         :    1;
        num_sges                         :    8;
        tbl_id                           :    8;
        rexmit_psn                       :   24;
        msn                              :   24;
        pad                              :   17;
    }
}

header_type req_rx_rrqsge_to_lkey_info_t {
    fields {
        sge_va                           :   64;
        sge_bytes                        :   16;
        is_atomic                        :    1;
        dma_cmd_eop                      :    1;
        dma_cmd_start_index              :    4;
        sge_index                        :    8;
        cq_dma_cmd_index                 :    8;
        cq_id                            :   24;
        rsvd_key_err                     :    1;
        bubble_one_stage                 :    1;
        pad                              :   32;
    }
}

header_type req_rx_sqcb1_to_timer_expiry_info_t {
    fields {
        rexmit_psn                       : 24;
    }
}

header_type req_rx_sqcb1_to_compl_feedback_info_t {
    fields {
        status                           : 8;
        err_qp_instantly                 : 1;
        ssn                              : 24;
    }
}

header_type req_rx_sqcb1_to_sq_drain_feedback_info_t {
    fields {
        ssn                               : 24;
    }
}

header_type req_rx_stats_info_t {
    fields {
        pad                              :  160;
    }
}

@pragma pa_header_union ingress app_header rdma_recirc rdma_bth rdma_bth_immeth rdma_bth_reth rdma_bth_reth_immeth rdma_bth_aeth rdma_bth_aeth_atomicaeth rdma_bth_atomiceth rdma_bth_ieth rdma_bth_deth rdma_bth_deth_immeth rdma_bth_xrceth rdma_bth_xrceth_immeth rdma_bth_xrceth_reth rdma_bth_xrceth_reth_immeth rdma_bth_xrceth_atomiceth rdma_bth_xrceth_ieth rdma_completion_feedback rdma_timer_expiry_feedback rdma_sq_drain_feedback

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
metadata rdma_timer_expiry_feedback_header_t rdma_timer_expiry_feedback;
metadata rdma_sq_drain_feedback_header_t rdma_sq_drain_feedback;


@pragma pa_header_union ingress ext_app_header rdma_bth_atomiceth_ext rdma_bth_xrceth_atomiceth_ext rdma_bth_xrceth_reth_immeth_ext rdma_bth_deth_immeth_ext

metadata p4_to_p4plus_roce_bth_atomiceth_ext_header_t rdma_bth_atomiceth_ext;
metadata p4_to_p4plus_roce_bth_xrceth_atomiceth_ext_header_t rdma_bth_xrceth_atomiceth_ext;
metadata p4_to_p4plus_roce_bth_xrceth_reth_immeth_ext_header_t rdma_bth_xrceth_reth_immeth_ext;
metadata p4_to_p4plus_roce_bth_deth_immeth_ext_header_t rdma_bth_deth_immeth_ext;

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
@pragma scratch_metadata
metadata rdma_timer_expiry_feedback_header_t rdma_timer_expiry_feedback_scr;
@pragma scratch_metadata
metadata rdma_sq_drain_feedback_header_t rdma_sq_drain_feedback_scr;


//Extended headers
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_atomiceth_ext_header_t rdma_bth_atomiceth_ext_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_xrceth_atomiceth_ext_header_t rdma_bth_xrceth_atomiceth_ext_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_xrceth_reth_immeth_ext_header_t rdma_bth_xrceth_reth_immeth_ext_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_deth_immeth_ext_header_t rdma_bth_deth_immeth_ext_scr;


/**** header unions and scratch ****/

/**** global header unions ****/

@pragma pa_header_union ingress common_global
metadata phv_global_common_t phv_global_common;
@pragma scratch_metadata
metadata phv_global_common_t phv_global_common_scr;

/**** to stage header unions ****/

@pragma pa_header_union ingress to_stage_1 to_s1_rrqwqe_info to_s1_recirc_info
metadata req_rx_to_stage_rrqwqe_info_t to_s1_rrqwqe_info;
@pragma scratch_metadata
metadata req_rx_to_stage_rrqwqe_info_t to_s1_rrqwqe_info_scr;

metadata req_rx_to_stage_recirc_info_t to_s1_recirc_info;
@pragma scratch_metadata
metadata req_rx_to_stage_recirc_info_t to_s1_recirc_info_scr;

@pragma pa_header_union ingress to_stage_2
metadata req_rx_to_stage_rrqsge_info_t to_s2_rrqsge_info;
@pragma scratch_metadata
metadata req_rx_to_stage_rrqsge_info_t to_s2_rrqsge_info_scr;

@pragma pa_header_union ingress to_stage_3
metadata req_rx_to_stage_rrqlkey_info_t to_s3_rrqlkey_info;
@pragma scratch_metadata
metadata req_rx_to_stage_rrqlkey_info_t to_s3_rrqlkey_info_scr;

@pragma pa_header_union ingress to_stage_4
metadata req_rx_to_stage_sqcb1_wb_info_t to_s4_sqcb1_wb_info;
@pragma scratch_metadata
metadata req_rx_to_stage_sqcb1_wb_info_t to_s4_sqcb1_wb_info_scr;

@pragma pa_header_union ingress to_stage_5
metadata req_rx_to_stage_cq_info_t to_s5_cq_info;
@pragma scratch_metadata
metadata req_rx_to_stage_cq_info_t to_s5_cq_info_scr;

@pragma pa_header_union ingress to_stage_6
metadata req_rx_to_stage_cq_info_t to_s6_cq_info;
@pragma scratch_metadata
metadata req_rx_to_stage_cq_info_t to_s6_cq_info_scr;

@pragma pa_header_union ingress to_stage_7
metadata req_rx_to_stage_stats_info_t to_s7_stats_info;
@pragma scratch_metadata
metadata req_rx_to_stage_stats_info_t to_s7_stats_info_scr;

/**** stage to stage header unions ****/

//Table-0

@pragma pa_header_union ingress common_t0_s2s t0_s2s_sqcb1_to_rrqwqe_info t0_s2s_rrqwqe_to_sge_info t0_s2s_rrqsge_to_lkey_info t0_s2s_rrqlkey_to_ptseg_info t0_s2s_sqcb1_to_sge_recirc_info t0_s2s_cqcb_to_eq_info

metadata req_rx_sqcb1_to_rrqwqe_info_t t0_s2s_sqcb1_to_rrqwqe_info;
@pragma scratch_metadata
metadata req_rx_sqcb1_to_rrqwqe_info_t t0_s2s_sqcb1_to_rrqwqe_info_scr;

metadata req_rx_sqcb1_to_sge_recirc_info_t t0_s2s_sqcb1_to_sge_recirc_info;
@pragma scratch_metadata
metadata req_rx_sqcb1_to_sge_recirc_info_t t0_s2s_sqcb1_to_sge_recirc_info_scr;

metadata req_rx_rrqwqe_to_sge_info_t t0_s2s_rrqwqe_to_sge_info;
@pragma scratch_metadata
metadata req_rx_rrqwqe_to_sge_info_t t0_s2s_rrqwqe_to_sge_info_scr;

metadata req_rx_rrqsge_to_lkey_info_t t0_s2s_rrqsge_to_lkey_info;
@pragma scratch_metadata
metadata req_rx_rrqsge_to_lkey_info_t t0_s2s_rrqsge_to_lkey_info_scr;

metadata req_rx_rrqlkey_to_ptseg_info_t t0_s2s_rrqlkey_to_ptseg_info;
@pragma scratch_metadata
metadata req_rx_rrqlkey_to_ptseg_info_t t0_s2s_rrqlkey_to_ptseg_info_scr;

metadata req_rx_cqcb_to_eq_info_t t0_s2s_cqcb_to_eq_info;
@pragma scratch_metadata
metadata req_rx_cqcb_to_eq_info_t t0_s2s_cqcb_to_eq_info_scr;

//Table-1

@pragma pa_header_union ingress common_t1_s2s t1_s2s_rrqsge_to_lkey_info t1_s2s_rrqlkey_to_ptseg_info t1_s2s_cqcb_to_eq_info
metadata req_rx_rrqsge_to_lkey_info_t t1_s2s_rrqsge_to_lkey_info;
@pragma scratch_metadata
metadata req_rx_rrqsge_to_lkey_info_t t1_s2s_rrqsge_to_lkey_info_scr;

metadata req_rx_rrqlkey_to_ptseg_info_t t1_s2s_rrqlkey_to_ptseg_info;
@pragma scratch_metadata
metadata req_rx_rrqlkey_to_ptseg_info_t t1_s2s_rrqlkey_to_ptseg_info_scr;

metadata req_rx_cqcb_to_eq_info_t t1_s2s_cqcb_to_eq_info;
@pragma scratch_metadata
metadata req_rx_cqcb_to_eq_info_t t1_s2s_cqcb_to_eq_info_scr;

//Table-2

@pragma pa_header_union ingress common_t2_s2s t2_s2s_rrqwqe_to_cq_info t2_s2s_sqcb1_write_back_info t2_s2s_sqcb1_to_timer_expiry_info t2_s2s_cqcb_to_pt_info t2_s2s_sqcb1_to_compl_feedback_info t2_s2s_sqcb1_to_sq_drain_feedback_info

metadata req_rx_rrqwqe_to_cq_info_t t2_s2s_rrqwqe_to_cq_info;
@pragma scratch_metadata
metadata req_rx_rrqwqe_to_cq_info_t t2_s2s_rrqwqe_to_cq_info_scr;

metadata req_rx_sqcb1_write_back_info_t t2_s2s_sqcb1_write_back_info;
@pragma scratch_metadata
metadata req_rx_sqcb1_write_back_info_t t2_s2s_sqcb1_write_back_info_scr;

metadata req_rx_sqcb1_to_timer_expiry_info_t t2_s2s_sqcb1_to_timer_expiry_info;
@pragma scratch_metadata
metadata req_rx_sqcb1_to_timer_expiry_info_t t2_s2s_sqcb1_to_timer_expiry_info_scr;

metadata req_rx_sqcb1_to_sq_drain_feedback_info_t t2_s2s_sqcb1_to_sq_drain_feedback_info;
@pragma scratch_metadata
metadata req_rx_sqcb1_to_sq_drain_feedback_info_t t2_s2s_sqcb1_to_sq_drain_feedback_info_scr;

metadata req_rx_cqcb_to_pt_info_t t2_s2s_cqcb_to_pt_info;
@pragma scratch_metadata
metadata req_rx_cqcb_to_pt_info_t t2_s2s_cqcb_to_pt_info_scr;

metadata req_rx_sqcb1_to_compl_feedback_info_t t2_s2s_sqcb1_to_compl_feedback_info;
@pragma scratch_metadata
metadata req_rx_sqcb1_to_compl_feedback_info_t t2_s2s_sqcb1_to_compl_feedback_info_scr;

//Table-3

@pragma pa_header_union ingress common_t3_s2s t3_s2s_ecn_info t3_s2s_stats_info

metadata req_rx_ecn_info_t t3_s2s_ecn_info;
@pragma scratch_metadata
metadata req_rx_ecn_info_t t3_s2s_ecn_info_scr;

metadata req_rx_stats_info_t t3_s2s_stats_info;
@pragma scratch_metadata
metadata req_rx_stats_info_t t3_s2s_stats_info_scr;

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
}

/*
 * Stage 0 table 0 bth deth immeth action
 */
action rdma_stage0_ext_bth_deth_immeth_action () {
    // k + i for stage 0

    modify_field(rdma_bth_deth_immeth_ext_scr.smac_2, rdma_bth_deth_immeth_ext.smac_2);
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
    modify_field(rdma_completion_feedback_scr.err_qp_instantly, rdma_completion_feedback.err_qp_instantly);
    modify_field(rdma_completion_feedback_scr.lif_cqe_error_id_vld, rdma_completion_feedback.lif_cqe_error_id_vld);
    modify_field(rdma_completion_feedback_scr.lif_error_id_vld, rdma_completion_feedback.lif_error_id_vld);
    modify_field(rdma_completion_feedback_scr.lif_error_id, rdma_completion_feedback.lif_error_id);
    modify_field(rdma_completion_feedback_scr.ssn, rdma_completion_feedback.ssn);
    modify_field(rdma_completion_feedback_scr.tx_psn, rdma_completion_feedback.tx_psn);
}


/*
 * Stage 0 table 0 timer_expiry_feedback action
 */
action rdma_stage0_timer_expiry_feedback_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_timer_expiry_feedback_scr.common_header_bits, rdma_timer_expiry_feedback.common_header_bits);

    // timer_expiry_feedback_header bits
    modify_field(rdma_timer_expiry_feedback_scr.feedback_type, rdma_timer_expiry_feedback.feedback_type);
    modify_field(rdma_timer_expiry_feedback_scr.rexmit_psn, rdma_timer_expiry_feedback.rexmit_psn);
    modify_field(rdma_timer_expiry_feedback_scr.ssn, rdma_timer_expiry_feedback.ssn);
    modify_field(rdma_timer_expiry_feedback_scr.tx_psn, rdma_timer_expiry_feedback.tx_psn);
}

/*
 * Stage 0 table 0 sq_drain_feedback action
 */
action rdma_stage0_sq_drain_feedback_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_sq_drain_feedback.common_header_bits, rdma_sq_drain_feedback.common_header_bits);

    // sq_drain_feedback_header bits
    modify_field(rdma_sq_drain_feedback_scr.feedback_type, rdma_sq_drain_feedback.feedback_type);
    modify_field(rdma_sq_drain_feedback_scr.ssn, rdma_sq_drain_feedback.ssn);
    modify_field(rdma_sq_drain_feedback_scr.tx_psn, rdma_sq_drain_feedback.tx_psn);
}

action req_rx_cqcb_process_s5 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s5_cq_info_scr.cqcb_base_addr_hi, to_s5_cq_info.cqcb_base_addr_hi);
    modify_field(to_s5_cq_info_scr.log_num_cq_entries, to_s5_cq_info.log_num_cq_entries);
    modify_field(to_s5_cq_info_scr.bth_se, to_s5_cq_info.bth_se);
    modify_field(to_s5_cq_info_scr.async_event, to_s5_cq_info.async_event);
    modify_field(to_s5_cq_info_scr.async_error_event, to_s5_cq_info.async_error_event);
    modify_field(to_s5_cq_info_scr.state, to_s5_cq_info.state);

    // stage to stage
    modify_field(t2_s2s_rrqwqe_to_cq_info_scr.cq_id, t2_s2s_rrqwqe_to_cq_info.cq_id);
    modify_field(t2_s2s_rrqwqe_to_cq_info_scr.cqe_type, t2_s2s_rrqwqe_to_cq_info.cqe_type);

}

action req_rx_cqcb_process_s6 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s6_cq_info_scr.cqcb_base_addr_hi, to_s6_cq_info.cqcb_base_addr_hi);
    modify_field(to_s6_cq_info_scr.log_num_cq_entries, to_s6_cq_info.log_num_cq_entries);
    modify_field(to_s6_cq_info_scr.bth_se, to_s6_cq_info.bth_se);
    modify_field(to_s6_cq_info_scr.async_event, to_s6_cq_info.async_event);
    modify_field(to_s6_cq_info_scr.async_error_event, to_s6_cq_info.async_error_event);
    modify_field(to_s6_cq_info_scr.state, to_s6_cq_info.state);

    // stage to stage
    modify_field(t2_s2s_rrqwqe_to_cq_info_scr.cq_id, t2_s2s_rrqwqe_to_cq_info.cq_id);
    modify_field(t2_s2s_rrqwqe_to_cq_info_scr.cqe_type, t2_s2s_rrqwqe_to_cq_info.cqe_type);

}
action req_rx_cqpt_process () {
    // from ki global
    GENERATE_GLOBAL_K

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
action req_rx_dcqcn_ecn_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t3_s2s_ecn_info_scr.p_key, t3_s2s_ecn_info.p_key);
    modify_field(t3_s2s_ecn_info_scr.pad, t3_s2s_ecn_info.pad);

}
action req_rx_eqcb_process_t0 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t0_s2s_cqcb_to_eq_info_scr.async_eq, t0_s2s_cqcb_to_eq_info.async_eq);
    modify_field(t0_s2s_cqcb_to_eq_info_scr.cmd_eop, t0_s2s_cqcb_to_eq_info.cmd_eop);
    modify_field(t0_s2s_cqcb_to_eq_info_scr.donot_reset_tbl_vld, t0_s2s_cqcb_to_eq_info.donot_reset_tbl_vld);
    modify_field(t0_s2s_cqcb_to_eq_info_scr.pad, t0_s2s_cqcb_to_eq_info.pad);

}
action req_rx_eqcb_process_t1 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t1_s2s_cqcb_to_eq_info_scr.async_eq, t1_s2s_cqcb_to_eq_info.async_eq);
    modify_field(t1_s2s_cqcb_to_eq_info_scr.cmd_eop, t1_s2s_cqcb_to_eq_info.cmd_eop);
    modify_field(t1_s2s_cqcb_to_eq_info_scr.donot_reset_tbl_vld, t1_s2s_cqcb_to_eq_info.donot_reset_tbl_vld);
    modify_field(t1_s2s_cqcb_to_eq_info_scr.pad, t1_s2s_cqcb_to_eq_info.pad);

}
action req_rx_rrqlkey_process_t0 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s3_rrqlkey_info_scr.pd, to_s3_rrqlkey_info.pd);

    // stage to stage
    modify_field(t0_s2s_rrqsge_to_lkey_info_scr.sge_va, t0_s2s_rrqsge_to_lkey_info.sge_va);
    modify_field(t0_s2s_rrqsge_to_lkey_info_scr.sge_bytes, t0_s2s_rrqsge_to_lkey_info.sge_bytes);
    modify_field(t0_s2s_rrqsge_to_lkey_info_scr.is_atomic, t0_s2s_rrqsge_to_lkey_info.is_atomic);
    modify_field(t0_s2s_rrqsge_to_lkey_info_scr.dma_cmd_eop, t0_s2s_rrqsge_to_lkey_info.dma_cmd_eop);
    modify_field(t0_s2s_rrqsge_to_lkey_info_scr.dma_cmd_start_index, t0_s2s_rrqsge_to_lkey_info.dma_cmd_start_index);
    modify_field(t0_s2s_rrqsge_to_lkey_info_scr.sge_index, t0_s2s_rrqsge_to_lkey_info.sge_index);
    modify_field(t0_s2s_rrqsge_to_lkey_info_scr.cq_dma_cmd_index, t0_s2s_rrqsge_to_lkey_info.cq_dma_cmd_index);
    modify_field(t0_s2s_rrqsge_to_lkey_info_scr.cq_id, t0_s2s_rrqsge_to_lkey_info.cq_id);
    modify_field(t0_s2s_rrqsge_to_lkey_info_scr.rsvd_key_err, t0_s2s_rrqsge_to_lkey_info.rsvd_key_err);
    modify_field(t0_s2s_rrqsge_to_lkey_info_scr.bubble_one_stage, t0_s2s_rrqsge_to_lkey_info.bubble_one_stage);
    modify_field(t0_s2s_rrqsge_to_lkey_info_scr.pad, t0_s2s_rrqsge_to_lkey_info.pad);

}
action req_rx_rrqlkey_process_t1 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s3_rrqlkey_info_scr.pd, to_s3_rrqlkey_info.pd);

    // stage to stage
    modify_field(t1_s2s_rrqsge_to_lkey_info_scr.sge_va, t1_s2s_rrqsge_to_lkey_info.sge_va);
    modify_field(t1_s2s_rrqsge_to_lkey_info_scr.sge_bytes, t1_s2s_rrqsge_to_lkey_info.sge_bytes);
    modify_field(t1_s2s_rrqsge_to_lkey_info_scr.is_atomic, t1_s2s_rrqsge_to_lkey_info.is_atomic);
    modify_field(t1_s2s_rrqsge_to_lkey_info_scr.dma_cmd_eop, t1_s2s_rrqsge_to_lkey_info.dma_cmd_eop);
    modify_field(t1_s2s_rrqsge_to_lkey_info_scr.dma_cmd_start_index, t1_s2s_rrqsge_to_lkey_info.dma_cmd_start_index);
    modify_field(t1_s2s_rrqsge_to_lkey_info_scr.sge_index, t1_s2s_rrqsge_to_lkey_info.sge_index);
    modify_field(t1_s2s_rrqsge_to_lkey_info_scr.cq_dma_cmd_index, t1_s2s_rrqsge_to_lkey_info.cq_dma_cmd_index);
    modify_field(t1_s2s_rrqsge_to_lkey_info_scr.cq_id, t1_s2s_rrqsge_to_lkey_info.cq_id);
    modify_field(t1_s2s_rrqsge_to_lkey_info_scr.rsvd_key_err, t1_s2s_rrqsge_to_lkey_info.rsvd_key_err);
    modify_field(t1_s2s_rrqsge_to_lkey_info_scr.pad, t1_s2s_rrqsge_to_lkey_info.pad);

}
action req_rx_rrqptseg_process_t0 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t0_s2s_rrqlkey_to_ptseg_info_scr.pt_offset, t0_s2s_rrqlkey_to_ptseg_info.pt_offset);
    modify_field(t0_s2s_rrqlkey_to_ptseg_info_scr.pt_bytes, t0_s2s_rrqlkey_to_ptseg_info.pt_bytes);
    modify_field(t0_s2s_rrqlkey_to_ptseg_info_scr.log_page_size, t0_s2s_rrqlkey_to_ptseg_info.log_page_size);
    modify_field(t0_s2s_rrqlkey_to_ptseg_info_scr.dma_cmd_start_index, t0_s2s_rrqlkey_to_ptseg_info.dma_cmd_start_index);
    modify_field(t0_s2s_rrqlkey_to_ptseg_info_scr.is_atomic, t0_s2s_rrqlkey_to_ptseg_info.is_atomic);
    modify_field(t0_s2s_rrqlkey_to_ptseg_info_scr.dma_cmd_eop, t0_s2s_rrqlkey_to_ptseg_info.dma_cmd_eop);
    modify_field(t0_s2s_rrqlkey_to_ptseg_info_scr.sge_index, t0_s2s_rrqlkey_to_ptseg_info.sge_index);
    modify_field(t0_s2s_rrqlkey_to_ptseg_info_scr.pad, t0_s2s_rrqlkey_to_ptseg_info.pad);

}
action req_rx_rrqptseg_process_t1 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t1_s2s_rrqlkey_to_ptseg_info_scr.pt_offset, t1_s2s_rrqlkey_to_ptseg_info.pt_offset);
    modify_field(t1_s2s_rrqlkey_to_ptseg_info_scr.pt_bytes, t1_s2s_rrqlkey_to_ptseg_info.pt_bytes);
    modify_field(t1_s2s_rrqlkey_to_ptseg_info_scr.log_page_size, t1_s2s_rrqlkey_to_ptseg_info.log_page_size);
    modify_field(t1_s2s_rrqlkey_to_ptseg_info_scr.dma_cmd_start_index, t1_s2s_rrqlkey_to_ptseg_info.dma_cmd_start_index);
    modify_field(t1_s2s_rrqlkey_to_ptseg_info_scr.is_atomic, t1_s2s_rrqlkey_to_ptseg_info.is_atomic);
    modify_field(t1_s2s_rrqlkey_to_ptseg_info_scr.dma_cmd_eop, t1_s2s_rrqlkey_to_ptseg_info.dma_cmd_eop);
    modify_field(t1_s2s_rrqlkey_to_ptseg_info_scr.sge_index, t1_s2s_rrqlkey_to_ptseg_info.sge_index);
    modify_field(t1_s2s_rrqlkey_to_ptseg_info_scr.pad, t1_s2s_rrqlkey_to_ptseg_info.pad);

}
action req_rx_rrqsge_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s2_rrqsge_info_scr.msg_psn, to_s2_rrqsge_info.msg_psn);
    modify_field(to_s2_rrqsge_info_scr.log_pmtu, to_s2_rrqsge_info.log_pmtu);
    modify_field(to_s2_rrqsge_info_scr.priv_oper_enable, to_s2_rrqsge_info.priv_oper_enable);

    // stage to stage
    modify_field(t0_s2s_rrqwqe_to_sge_info_scr.remaining_payload_bytes, t0_s2s_rrqwqe_to_sge_info.remaining_payload_bytes);
    modify_field(t0_s2s_rrqwqe_to_sge_info_scr.cur_sge_offset, t0_s2s_rrqwqe_to_sge_info.cur_sge_offset);
    modify_field(t0_s2s_rrqwqe_to_sge_info_scr.cur_sge_id, t0_s2s_rrqwqe_to_sge_info.cur_sge_id);
    modify_field(t0_s2s_rrqwqe_to_sge_info_scr.num_valid_sges, t0_s2s_rrqwqe_to_sge_info.num_valid_sges);
    modify_field(t0_s2s_rrqwqe_to_sge_info_scr.rrq_in_progress, t0_s2s_rrqwqe_to_sge_info.rrq_in_progress);
    modify_field(t0_s2s_rrqwqe_to_sge_info_scr.is_atomic, t0_s2s_rrqwqe_to_sge_info.is_atomic);
    modify_field(t0_s2s_rrqwqe_to_sge_info_scr.dma_cmd_eop, t0_s2s_rrqwqe_to_sge_info.dma_cmd_eop);
    modify_field(t0_s2s_rrqwqe_to_sge_info_scr.dma_cmd_start_index, t0_s2s_rrqwqe_to_sge_info.dma_cmd_start_index);
    modify_field(t0_s2s_rrqwqe_to_sge_info_scr.post_cq, t0_s2s_rrqwqe_to_sge_info.post_cq);
    modify_field(t0_s2s_rrqwqe_to_sge_info_scr.end_of_page, t0_s2s_rrqwqe_to_sge_info.end_of_page);

}
action req_rx_rrqwqe_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_rrqwqe_info_scr.aeth_msn, to_s1_rrqwqe_info.aeth_msn);
    modify_field(to_s1_rrqwqe_info_scr.bth_psn, to_s1_rrqwqe_info.bth_psn);
    modify_field(to_s1_rrqwqe_info_scr.aeth_syndrome, to_s1_rrqwqe_info.aeth_syndrome);
    modify_field(to_s1_rrqwqe_info_scr.remaining_payload_bytes, to_s1_rrqwqe_info.remaining_payload_bytes);
    modify_field(to_s1_rrqwqe_info_scr.sge_opt, to_s1_rrqwqe_info.sge_opt);
    modify_field(to_s1_rrqwqe_info_scr.priv_oper_enable, to_s1_rrqwqe_info.priv_oper_enable);
    modify_field(to_s1_rrqwqe_info_scr.log_pmtu, to_s1_rrqwqe_info.log_pmtu);
    modify_field(to_s1_rrqwqe_info_scr.log_page_size, to_s1_rrqwqe_info.log_page_size);

    // stage to stage
    modify_field(t0_s2s_sqcb1_to_rrqwqe_info_scr.cur_sge_offset, t0_s2s_sqcb1_to_rrqwqe_info.cur_sge_offset);
    modify_field(t0_s2s_sqcb1_to_rrqwqe_info_scr.ssn, t0_s2s_sqcb1_to_rrqwqe_info.ssn);
    modify_field(t0_s2s_sqcb1_to_rrqwqe_info_scr.cur_sge_id, t0_s2s_sqcb1_to_rrqwqe_info.cur_sge_id);
    modify_field(t0_s2s_sqcb1_to_rrqwqe_info_scr.msg_psn, t0_s2s_sqcb1_to_rrqwqe_info.msg_psn);
    modify_field(t0_s2s_sqcb1_to_rrqwqe_info_scr.msn, t0_s2s_sqcb1_to_rrqwqe_info.msn);
    modify_field(t0_s2s_sqcb1_to_rrqwqe_info_scr.rrq_in_progress, t0_s2s_sqcb1_to_rrqwqe_info.rrq_in_progress);
    modify_field(t0_s2s_sqcb1_to_rrqwqe_info_scr.rrq_empty, t0_s2s_sqcb1_to_rrqwqe_info.rrq_empty);
    modify_field(t0_s2s_sqcb1_to_rrqwqe_info_scr.timer_active, t0_s2s_sqcb1_to_rrqwqe_info.timer_active);
    modify_field(t0_s2s_sqcb1_to_rrqwqe_info_scr.dma_cmd_start_index, t0_s2s_sqcb1_to_rrqwqe_info.dma_cmd_start_index);
    modify_field(t0_s2s_sqcb1_to_rrqwqe_info_scr.rrq_cindex, t0_s2s_sqcb1_to_rrqwqe_info.rrq_cindex);
    modify_field(t0_s2s_sqcb1_to_rrqwqe_info_scr.tx_psn, t0_s2s_sqcb1_to_rrqwqe_info.tx_psn);

}

action req_rx_sqcb1_recirc_process () {
    // from ki global
    GENERATE_GLOBAL_K

    modify_field(to_s1_recirc_info_scr.rrqwqe_sge_list_addr, to_s1_recirc_info.rrqwqe_sge_list_addr);
    modify_field(to_s1_recirc_info_scr.cur_sge_offset, to_s1_recirc_info.cur_sge_offset);
    modify_field(to_s1_recirc_info_scr.cur_sge_id, to_s1_recirc_info.cur_sge_id);
    modify_field(to_s1_recirc_info_scr.remaining_payload_bytes, to_s1_recirc_info.remaining_payload_bytes);
    modify_field(to_s1_recirc_info_scr.num_valid_sges, to_s1_recirc_info.num_valid_sges);
    modify_field(to_s1_recirc_info_scr.dma_cmd_eop, to_s1_recirc_info.dma_cmd_eop);
    modify_field(to_s1_recirc_info_scr.sge_opt, to_s1_recirc_info.sge_opt);

    modify_field(t0_s2s_sqcb1_to_sge_recirc_info_scr.rrq_in_progress, t0_s2s_sqcb1_to_sge_recirc_info.rrq_in_progress);
    modify_field(t0_s2s_sqcb1_to_sge_recirc_info_scr.log_page_size, t0_s2s_sqcb1_to_sge_recirc_info.log_page_size);
}

action req_rx_sqcb1_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage

}
action req_rx_sqcb1_write_back_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_sqcb1_wb_info_scr.remaining_payload_bytes, to_s4_sqcb1_wb_info.remaining_payload_bytes);
    modify_field(to_s4_sqcb1_wb_info_scr.my_token_id, to_s4_sqcb1_wb_info.my_token_id);
    modify_field(to_s4_sqcb1_wb_info_scr.error_disable_qp, to_s4_sqcb1_wb_info.error_disable_qp);
    modify_field(to_s4_sqcb1_wb_info_scr.error_drop_phv, to_s4_sqcb1_wb_info.error_drop_phv);
    modify_field(to_s4_sqcb1_wb_info_scr.sge_opt, to_s4_sqcb1_wb_info.sge_opt);
    modify_field(to_s4_sqcb1_wb_info_scr.rrq_spec_cindex, to_s4_sqcb1_wb_info.rrq_spec_cindex);

    // stage to stage
    modify_field(t2_s2s_sqcb1_write_back_info_scr.cur_sge_offset, t2_s2s_sqcb1_write_back_info.cur_sge_offset);
    modify_field(t2_s2s_sqcb1_write_back_info_scr.msg_psn, t2_s2s_sqcb1_write_back_info.msg_psn);
    modify_field(t2_s2s_sqcb1_write_back_info_scr.cur_sge_id, t2_s2s_sqcb1_write_back_info.cur_sge_id);
    modify_field(t2_s2s_sqcb1_write_back_info_scr.rrq_in_progress, t2_s2s_sqcb1_write_back_info.rrq_in_progress);
    modify_field(t2_s2s_sqcb1_write_back_info_scr.incr_nxt_to_go_token_id, t2_s2s_sqcb1_write_back_info.incr_nxt_to_go_token_id);
    modify_field(t2_s2s_sqcb1_write_back_info_scr.post_bktrack, t2_s2s_sqcb1_write_back_info.post_bktrack);
    modify_field(t2_s2s_sqcb1_write_back_info_scr.last_pkt, t2_s2s_sqcb1_write_back_info.last_pkt);
    modify_field(t2_s2s_sqcb1_write_back_info_scr.num_sges, t2_s2s_sqcb1_write_back_info.num_sges);
    modify_field(t2_s2s_sqcb1_write_back_info_scr.tbl_id, t2_s2s_sqcb1_write_back_info.tbl_id);
    modify_field(t2_s2s_sqcb1_write_back_info_scr.rexmit_psn, t2_s2s_sqcb1_write_back_info.rexmit_psn);
    modify_field(t2_s2s_sqcb1_write_back_info_scr.msn, t2_s2s_sqcb1_write_back_info.msn);
    modify_field(t2_s2s_sqcb1_write_back_info_scr.post_cq, t2_s2s_sqcb1_write_back_info.post_cq);
    modify_field(t2_s2s_sqcb1_write_back_info_scr.pad, t2_s2s_sqcb1_write_back_info.pad);
}

action req_rx_timer_expiry_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t2_s2s_sqcb1_to_timer_expiry_info_scr.rexmit_psn, t2_s2s_sqcb1_to_timer_expiry_info.rexmit_psn);

}

action req_rx_completion_feedback_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t2_s2s_sqcb1_to_compl_feedback_info_scr.status, t2_s2s_sqcb1_to_compl_feedback_info.status);
    modify_field(t2_s2s_sqcb1_to_compl_feedback_info_scr.err_qp_instantly, t2_s2s_sqcb1_to_compl_feedback_info.err_qp_instantly);
    modify_field(t2_s2s_sqcb1_to_compl_feedback_info_scr.ssn, t2_s2s_sqcb1_to_compl_feedback_info.ssn);

}

action req_rx_sq_drain_feedback_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t2_s2s_sqcb1_to_sq_drain_feedback_info_scr.ssn, t2_s2s_sqcb1_to_sq_drain_feedback_info.ssn);

}

action req_rx_stats_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s7_stats_info_scr.pyld_bytes, to_s7_stats_info.pyld_bytes);
    modify_field(to_s7_stats_info_scr.lif_cqe_error_id_vld, to_s7_stats_info.lif_cqe_error_id_vld);
    modify_field(to_s7_stats_info_scr.lif_error_id_vld, to_s7_stats_info.lif_error_id_vld);
    modify_field(to_s7_stats_info_scr.lif_error_id, to_s7_stats_info.lif_error_id);
    modify_field(to_s7_stats_info_scr.qp_err_disabled, to_s7_stats_info.qp_err_disabled);
    modify_field(to_s7_stats_info_scr.qp_err_dis_rrqlkey_pd_mismatch, to_s7_stats_info.qp_err_dis_rrqlkey_pd_mismatch);
    modify_field(to_s7_stats_info_scr.qp_err_dis_rrqlkey_inv_state, to_s7_stats_info.qp_err_dis_rrqlkey_inv_state);
    modify_field(to_s7_stats_info_scr.qp_err_dis_rrqlkey_rsvd_lkey, to_s7_stats_info.qp_err_dis_rrqlkey_rsvd_lkey);
    modify_field(to_s7_stats_info_scr.qp_err_dis_rrqlkey_acc_no_wr_perm, to_s7_stats_info.qp_err_dis_rrqlkey_acc_no_wr_perm);
    modify_field(to_s7_stats_info_scr.qp_err_dis_rrqlkey_acc_len_lower, to_s7_stats_info.qp_err_dis_rrqlkey_acc_len_lower);
    modify_field(to_s7_stats_info_scr.qp_err_dis_rrqlkey_acc_len_higher, to_s7_stats_info.qp_err_dis_rrqlkey_acc_len_higher);
    modify_field(to_s7_stats_info_scr.qp_err_dis_rrqsge_insuff_sges, to_s7_stats_info.qp_err_dis_rrqsge_insuff_sges);
    modify_field(to_s7_stats_info_scr.qp_err_dis_rrqsge_insuff_sge_len, to_s7_stats_info.qp_err_dis_rrqsge_insuff_sge_len);
    modify_field(to_s7_stats_info_scr.qp_err_dis_rrqsge_insuff_dma_cmds, to_s7_stats_info.qp_err_dis_rrqsge_insuff_dma_cmds);
    modify_field(to_s7_stats_info_scr.qp_err_dis_rrqwqe_remote_inv_req_err_rcvd, to_s7_stats_info.qp_err_dis_rrqwqe_remote_inv_req_err_rcvd);
    modify_field(to_s7_stats_info_scr.qp_err_dis_rrqwqe_remote_acc_err_rcvd, to_s7_stats_info.qp_err_dis_rrqwqe_remote_acc_err_rcvd);
    modify_field(to_s7_stats_info_scr.qp_err_dis_rrqwqe_remote_oper_err_rcvd, to_s7_stats_info.qp_err_dis_rrqwqe_remote_oper_err_rcvd);
    modify_field(to_s7_stats_info_scr.qp_err_dis_table_error, to_s7_stats_info.qp_err_dis_table_error);
    modify_field(to_s7_stats_info_scr.qp_err_dis_phv_intrinsic_error, to_s7_stats_info.qp_err_dis_phv_intrinsic_error);
    modify_field(to_s7_stats_info_scr.qp_err_dis_table_resp_error, to_s7_stats_info.qp_err_dis_table_resp_error);
    modify_field(to_s7_stats_info_scr.qp_err_dis_rsvd, to_s7_stats_info.qp_err_dis_rsvd);
    modify_field(to_s7_stats_info_scr.max_recirc_cnt_err, to_s7_stats_info.max_recirc_cnt_err);
    modify_field(to_s7_stats_info_scr.recirc_reason, to_s7_stats_info.recirc_reason);
    modify_field(to_s7_stats_info_scr.recirc_bth_opcode, to_s7_stats_info.recirc_bth_opcode);
    modify_field(to_s7_stats_info_scr.recirc_bth_psn, to_s7_stats_info.recirc_bth_psn); 
    modify_field(to_s7_stats_info_scr.pad, to_s7_stats_info.pad);

    // stage to stage
    modify_field(t3_s2s_stats_info_scr.pad, t3_s2s_stats_info.pad);

}

