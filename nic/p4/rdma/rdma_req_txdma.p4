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
#define tx_table_s0_t2_action req_tx_sqsge_process_recirc

#define tx_table_s1_t0_action req_tx_dummy_sqpt_process
#define tx_table_s1_t0_action1 req_tx_sqpt_process
#define tx_table_s1_t0_action3 req_tx_bktrack_sqcb2_process
#define tx_table_s1_t0_action4 req_tx_timer_process
#define tx_table_s1_t0_action4 req_tx_sqcb2_cnp_process

#define tx_table_s1_t2_action req_tx_sqsge_iterate_process

#define tx_table_s2_t0_action  req_tx_sqwqe_process
#define tx_table_s2_t0_action1 req_tx_bktrack_sqwqe_process_s2
#define tx_table_s2_t0_action2 req_tx_credits_process
#define tx_table_s2_t0_action3 req_tx_bktrack_write_back_process_s2
#define tx_table_s2_t0_action4 req_tx_bktrack_sqpt_process
#define tx_table_s2_t0_action6 req_tx_dcqcn_cnp_process
#define tx_table_s2_t1_action  req_tx_bktrack_sqcb1_write_back_process

#define tx_table_s3_t0_action  req_tx_sqsge_process 
#define tx_table_s3_t0_action1 req_tx_bktrack_sqwqe_process_s3
#define tx_table_s3_t0_action2 req_tx_bktrack_sqsge_process_s3
#define tx_table_s3_t0_action3 req_tx_bktrack_write_back_process_s3
#define tx_table_s3_t1_action4 req_tx_bktrack_sqcb2_write_back_process
#define tx_table_s3_t2_action  req_tx_dcqcn_enforce_process_s3

#define tx_table_s4_t0_action  req_tx_sqlkey_process_t0
#define tx_table_s4_t0_action1 req_tx_bktrack_sqwqe_process_s4
#define tx_table_s4_t0_action2 req_tx_bktrack_sqsge_process_s4
#define tx_table_s4_t0_action3 req_tx_bktrack_write_back_process_s4
#define tx_table_s4_t1_action  req_tx_sqlkey_process_t1
#define tx_table_s4_t1_action4 req_tx_bktrack_sqcb2_write_back_process
#define tx_table_s4_t2_action  req_tx_dcqcn_enforce_process_s4

#define tx_table_s5_t0_action  req_tx_sqptseg_process_t0
#define tx_table_s5_t0_action1 req_tx_bktrack_sqwqe_process_s5
#define tx_table_s5_t0_action2 req_tx_bktrack_sqsge_process_s5
#define tx_table_s5_t0_action3 req_tx_bktrack_write_back_process_s5
#define tx_table_s5_t1_action  req_tx_sqptseg_process_t1
#define tx_table_s5_t1_action1 req_tx_load_hdr_template_process
#define tx_table_s5_t1_action4 req_tx_bktrack_sqcb2_write_back_process
#define tx_table_s5_t2_action  req_tx_write_back_process
#define tx_table_s5_t2_action1 req_tx_write_back_process_rd
#define tx_table_s5_t2_action2 req_tx_write_back_process_send_wr
#define tx_table_s5_t3_action  req_tx_add_headers_process_rd
#define tx_table_s5_t3_action1 req_tx_add_headers_process_send_wr

#define tx_table_s6_t0_action1 req_tx_bktrack_sqwqe_process_s6
#define tx_table_s6_t0_action2 req_tx_bktrack_sqsge_process_s6
#define tx_table_s6_t0_action3 req_tx_bktrack_write_back_process_s6
#define tx_table_s6_t1_action4 req_tx_bktrack_sqcb2_write_back_process
#define tx_table_s6_t3_action  req_tx_add_headers_2_process

#define tx_table_s7_t0_action3 req_tx_bktrack_write_back_process_s7
#define tx_table_s7_t1_action4 req_tx_bktrack_sqcb2_write_back_process

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
    modify_field(phv_global_common_scr.pad, phv_global_common.pad);\
    modify_field(phv_global_common_scr.error_disable_qp, phv_global_common.error_disable_qp);\
    modify_field(phv_global_common_scr.ud_service, phv_global_common.ud_service);\

/**** header definitions ****/

header_type req_tx_sqcb1_to_credits_info_t {
    fields {
        need_credits                     :    1;
        pad                              :  159;
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
        pad                              :   23;
        error_disable_qp                 :    1;
        ud_service                       :    1;
    }
}

header_type req_tx_bktrack_sqcb2_write_back_info_t {
    fields {
        tx_psn                           :   24;
        ssn                              :   24;
        skip_wqe_start_psn               :    1;
        imm_data                         :   32;
        inv_key                          :   32;
        op_type                          :    4;
        sq_cindex                        :   16;
        in_progress                      :    1;
        pad                              :   26;
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

header_type req_tx_bktrack_to_stage_t {
    fields {
        wqe_addr                         :   64;
        log_pmtu                         :    5;
        log_sq_page_size                 :    5;
        log_wqe_size                     :    5;
        log_num_wqes                     :    5;
        rexmit_psn                       :   24;
        pad                              :   20;
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
        ah_size                          : 8;
        poll_in_progress                 : 1;
        color                            : 1;
        rsvd                             : 10;
        dma_cmd_start_index              : 6;
        imm_data                         : 32;
        inv_key_or_ah_handle             : 32;
    }
}

header_type req_tx_sq_bktrack_info_t {
    fields {
        sq_c_index                          : 16;
        in_progress                         : 1;
        current_sge_offset                  : 32;
        current_sge_id                      : 8;
        num_sges                            : 8;
        tx_psn                              : 24;
        ssn                                 : 24;
        op_type                             : 4;
        sq_p_index_or_imm_data1_or_inv_key1 : 16;
        imm_data2_or_inv_key2               : 16;
        pad                                 : 11;
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
        current_sge_offset               :   32;
        current_sge_id                   :    8;
        num_sges                         :    8;
        update_credits                   :    1;
        bktrack                          :    1;
        pt_base_addr                     :   32;
        op_type                          :    4;
        sq_in_hbm                        :    1;
        pad                              :   39;
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
        rsvd                             :    6;
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
        rsvd                             :    8;
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
        rsvd                             :    8;
        op_send_wr_imm_data              :   32;
        op_send_wr_inv_key_or_ah_handle  :   32;
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
        pad                              :   47;
    }
}

header_type req_tx_sq_to_stage_t {
    fields {
        wqe_addr                         :   64;
        spec_cindex                      :   16;
        header_template_addr             :   32;
        packet_len                       :   14;
        congestion_mgmt_enable           :    1;
        rate_enforce_failed              :    1;
    }
}

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
        remaining_payload_bytes          :   32;
        rrq_p_index                      :    8;
        pd                               :   32;
        log_pmtu                         :    5;
        poll_in_progress                 :    1;
        color                            :    1;
        pad                              :   62;
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
        pad                              :   91;
    }
}

header_type req_tx_sge_to_lkey_info_t {
    fields {
        sge_va                           :   64;
        sge_bytes                        :   16;
        dma_cmd_start_index              :    8;
        sge_index                        :    8;
        pad                              :   64;
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
@pragma pa_header_union ingress to_stage_0 to_s0_sq_to_stage

metadata req_tx_sq_to_stage_t to_s0_sq_to_stage;
@pragma scratch_metadata
metadata req_tx_sq_to_stage_t to_s0_sq_to_stage_scr;


//To-Stage-1
@pragma pa_header_union ingress to_stage_1 to_s1_bktrack_to_stage to_s1_sq_to_stage

metadata req_tx_bktrack_to_stage_t to_s1_bktrack_to_stage;
@pragma scratch_metadata
metadata req_tx_bktrack_to_stage_t to_s1_bktrack_to_stage_scr;

metadata req_tx_sq_to_stage_t to_s1_sq_to_stage;
@pragma scratch_metadata
metadata req_tx_sq_to_stage_t to_s1_sq_to_stage_scr;

//To-Stage-2
@pragma pa_header_union ingress to_stage_2 to_s2_sq_to_stage to_s2_bktrack_to_stage

metadata req_tx_sq_to_stage_t to_s2_sq_to_stage;
@pragma scratch_metadata
metadata req_tx_sq_to_stage_t to_s2_sq_to_stage_scr;

metadata req_tx_bktrack_to_stage_t to_s2_bktrack_to_stage;
@pragma scratch_metadata
metadata req_tx_bktrack_to_stage_t to_s2_bktrack_to_stage_scr;

//To-Stage-3
@pragma pa_header_union ingress to_stage_3 to_s3_bktrack_to_stage to_s3_sq_to_stage

metadata req_tx_bktrack_to_stage_t to_s3_bktrack_to_stage;
@pragma scratch_metadata
metadata req_tx_bktrack_to_stage_t to_s3_bktrack_to_stage_scr;

metadata req_tx_sq_to_stage_t to_s3_sq_to_stage;
@pragma scratch_metadata
metadata req_tx_sq_to_stage_t to_s3_sq_to_stage_scr;

//To-Stage-4
@pragma pa_header_union ingress to_stage_4 to_s4_bktrack_to_stage to_s4_sq_to_stage

metadata req_tx_bktrack_to_stage_t to_s4_bktrack_to_stage;
@pragma scratch_metadata
metadata req_tx_bktrack_to_stage_t to_s4_bktrack_to_stage_scr;

metadata req_tx_sq_to_stage_t to_s4_sq_to_stage;
@pragma scratch_metadata
metadata req_tx_sq_to_stage_t to_s4_sq_to_stage_scr;

//To-Stage-5
@pragma pa_header_union ingress to_stage_5 to_s5_bktrack_to_stage to_s5_sq_to_stage

metadata req_tx_bktrack_to_stage_t to_s5_bktrack_to_stage;
@pragma scratch_metadata
metadata req_tx_bktrack_to_stage_t to_s5_bktrack_to_stage_scr;

metadata req_tx_sq_to_stage_t to_s5_sq_to_stage;
@pragma scratch_metadata
metadata req_tx_sq_to_stage_t to_s5_sq_to_stage_scr;

//To-Stage-6
@pragma pa_header_union ingress to_stage_6 to_s6_bktrack_to_stage

metadata req_tx_bktrack_to_stage_t to_s6_bktrack_to_stage;
@pragma scratch_metadata
metadata req_tx_bktrack_to_stage_t to_s6_bktrack_to_stage_scr;

//To-Stage-7
@pragma pa_header_union ingress to_stage_7 to_s7_bktrack_to_stage

metadata req_tx_bktrack_to_stage_t to_s7_bktrack_to_stage;
@pragma scratch_metadata
metadata req_tx_bktrack_to_stage_t to_s7_bktrack_to_stage_scr;

/**** stage to stage header unions ****/

//Table-0
@pragma pa_header_union ingress common_t0_s2s t0_s2s_sqcb_to_wqe_info t0_s2s_sqcb_to_pt_info t0_s2s_sqcb0_to_sqcb2_info t0_s2s_wqe_to_sge_info t0_s2s_sqcb1_to_credits_info t0_s2s_sq_bktrack_info t0_s2s_sqcb_write_back_info t0_s2s_sge_to_lkey_info t0_s2s_lkey_to_ptseg_info

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

metadata req_tx_sqcb1_to_credits_info_t t0_s2s_sqcb1_to_credits_info;
@pragma scratch_metadata
metadata req_tx_sqcb1_to_credits_info_t t0_s2s_sqcb1_to_credits_info_scr;

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

//Table-1
@pragma pa_header_union ingress common_t1_s2s t1_s2s_bktrack_sqcb2_write_back_info t1_s2s_sge_to_lkey_info t1_s2s_lkey_to_ptseg_info t1_s2s_bktrack_sqcb1_write_back_info

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

//Table-2
@pragma pa_header_union ingress common_t2_s2s t2_s2s_wqe_to_sge_info t2_s2s_sqcb_write_back_info t2_s2s_sqcb_write_back_info_rd t2_s2s_sqcb_write_back_info_send_wr

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

//Table-3
@pragma pa_header_union ingress common_t3_s2s t3_s2s_sqcb_write_back_info_rd t3_s2s_sqcb_write_back_info_send_wr t3_s2s_add_hdr_info

metadata req_tx_sqcb_write_back_info_rd_t t3_s2s_sqcb_write_back_info_rd;
@pragma scratch_metadata
metadata req_tx_sqcb_write_back_info_rd_t t3_s2s_sqcb_write_back_info_rd_scr;

metadata req_tx_sqcb_write_back_info_send_wr_t t3_s2s_sqcb_write_back_info_send_wr;
@pragma scratch_metadata
metadata req_tx_sqcb_write_back_info_send_wr_t t3_s2s_sqcb_write_back_info_send_wr_scr;

metadata req_tx_add_hdr_info_t t3_s2s_add_hdr_info;
@pragma scratch_metadata
metadata req_tx_add_hdr_info_t t3_s2s_add_hdr_info_scr;

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
}


action req_tx_bktrack_sqcb2_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_bktrack_to_stage_scr.wqe_addr, to_s1_bktrack_to_stage.wqe_addr);
    modify_field(to_s1_bktrack_to_stage_scr.log_pmtu, to_s1_bktrack_to_stage.log_pmtu);
    modify_field(to_s1_bktrack_to_stage_scr.log_sq_page_size, to_s1_bktrack_to_stage.log_sq_page_size);
    modify_field(to_s1_bktrack_to_stage_scr.log_wqe_size, to_s1_bktrack_to_stage.log_wqe_size);
    modify_field(to_s1_bktrack_to_stage_scr.log_num_wqes, to_s1_bktrack_to_stage.log_num_wqes);
    modify_field(to_s1_bktrack_to_stage_scr.rexmit_psn, to_s1_bktrack_to_stage.rexmit_psn);
    modify_field(to_s1_bktrack_to_stage_scr.pad, to_s1_bktrack_to_stage.pad);

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
    modify_field(t0_s2s_sqcb0_to_sqcb2_info_scr.pad, t0_s2s_sqcb0_to_sqcb2_info.pad);

}
action req_tx_timer_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_bktrack_to_stage_scr.wqe_addr, to_s1_bktrack_to_stage.wqe_addr);
    modify_field(to_s1_bktrack_to_stage_scr.log_pmtu, to_s1_bktrack_to_stage.log_pmtu);
    modify_field(to_s1_bktrack_to_stage_scr.log_sq_page_size, to_s1_bktrack_to_stage.log_sq_page_size);
    modify_field(to_s1_bktrack_to_stage_scr.log_wqe_size, to_s1_bktrack_to_stage.log_wqe_size);
    modify_field(to_s1_bktrack_to_stage_scr.log_num_wqes, to_s1_bktrack_to_stage.log_num_wqes);
    modify_field(to_s1_bktrack_to_stage_scr.rexmit_psn, to_s1_bktrack_to_stage.rexmit_psn);
    modify_field(to_s1_bktrack_to_stage_scr.pad, to_s1_bktrack_to_stage.pad);

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
    modify_field(t0_s2s_sqcb0_to_sqcb2_info_scr.pad, t0_s2s_sqcb0_to_sqcb2_info.pad);

}
action req_tx_bktrack_sqcb1_write_back_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t1_s2s_bktrack_sqcb1_write_back_info_scr.tx_psn, t1_s2s_bktrack_sqcb1_write_back_info.tx_psn);
    modify_field(t1_s2s_bktrack_sqcb1_write_back_info_scr.ssn, t1_s2s_bktrack_sqcb1_write_back_info.ssn);
}
action req_tx_bktrack_sqcb2_write_back_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t1_s2s_bktrack_sqcb2_write_back_info_scr.tx_psn, t1_s2s_bktrack_sqcb2_write_back_info.tx_psn);
    modify_field(t1_s2s_bktrack_sqcb2_write_back_info_scr.ssn, t1_s2s_bktrack_sqcb2_write_back_info.ssn);
    modify_field(t1_s2s_bktrack_sqcb2_write_back_info_scr.skip_wqe_start_psn, t1_s2s_bktrack_sqcb2_write_back_info.skip_wqe_start_psn);
    modify_field(t1_s2s_bktrack_sqcb2_write_back_info_scr.imm_data, t1_s2s_bktrack_sqcb2_write_back_info.imm_data);
    modify_field(t1_s2s_bktrack_sqcb2_write_back_info_scr.inv_key, t1_s2s_bktrack_sqcb2_write_back_info.inv_key);
    modify_field(t1_s2s_bktrack_sqcb2_write_back_info_scr.op_type, t1_s2s_bktrack_sqcb2_write_back_info.op_type);
    modify_field(t1_s2s_bktrack_sqcb2_write_back_info_scr.sq_cindex, t1_s2s_bktrack_sqcb2_write_back_info.sq_cindex);
    modify_field(t1_s2s_bktrack_sqcb2_write_back_info_scr.in_progress, t1_s2s_bktrack_sqcb2_write_back_info.in_progress);
    modify_field(t1_s2s_bktrack_sqcb2_write_back_info_scr.pad, t1_s2s_bktrack_sqcb2_write_back_info.pad);

}
action req_tx_bktrack_sqpt_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s2_bktrack_to_stage_scr.wqe_addr, to_s2_bktrack_to_stage.wqe_addr);
    modify_field(to_s2_bktrack_to_stage_scr.log_pmtu, to_s2_bktrack_to_stage.log_pmtu);
    modify_field(to_s2_bktrack_to_stage_scr.log_sq_page_size, to_s2_bktrack_to_stage.log_sq_page_size);
    modify_field(to_s2_bktrack_to_stage_scr.log_wqe_size, to_s2_bktrack_to_stage.log_wqe_size);
    modify_field(to_s2_bktrack_to_stage_scr.log_num_wqes, to_s2_bktrack_to_stage.log_num_wqes);
    modify_field(to_s2_bktrack_to_stage_scr.rexmit_psn, to_s2_bktrack_to_stage.rexmit_psn);
    modify_field(to_s2_bktrack_to_stage_scr.pad, to_s2_bktrack_to_stage.pad);

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
    modify_field(t0_s2s_sq_bktrack_info_scr.pad, t0_s2s_sq_bktrack_info.pad);
}

action req_tx_bktrack_sqsge_process_s3 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s3_bktrack_to_stage_scr.wqe_addr, to_s3_bktrack_to_stage.wqe_addr);
    modify_field(to_s3_bktrack_to_stage_scr.log_pmtu, to_s3_bktrack_to_stage.log_pmtu);
    modify_field(to_s3_bktrack_to_stage_scr.log_sq_page_size, to_s3_bktrack_to_stage.log_sq_page_size);
    modify_field(to_s3_bktrack_to_stage_scr.log_wqe_size, to_s3_bktrack_to_stage.log_wqe_size);
    modify_field(to_s3_bktrack_to_stage_scr.log_num_wqes, to_s3_bktrack_to_stage.log_num_wqes);
    modify_field(to_s3_bktrack_to_stage_scr.rexmit_psn, to_s3_bktrack_to_stage.rexmit_psn);
    modify_field(to_s3_bktrack_to_stage_scr.pad, to_s3_bktrack_to_stage.pad);

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
    modify_field(t0_s2s_sq_bktrack_info_scr.pad, t0_s2s_sq_bktrack_info.pad);

}

action req_tx_bktrack_sqsge_process_s4 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_bktrack_to_stage_scr.wqe_addr, to_s4_bktrack_to_stage.wqe_addr);
    modify_field(to_s4_bktrack_to_stage_scr.log_pmtu, to_s4_bktrack_to_stage.log_pmtu);
    modify_field(to_s4_bktrack_to_stage_scr.log_sq_page_size, to_s4_bktrack_to_stage.log_sq_page_size);
    modify_field(to_s4_bktrack_to_stage_scr.log_wqe_size, to_s4_bktrack_to_stage.log_wqe_size);
    modify_field(to_s4_bktrack_to_stage_scr.log_num_wqes, to_s4_bktrack_to_stage.log_num_wqes);
    modify_field(to_s4_bktrack_to_stage_scr.rexmit_psn, to_s4_bktrack_to_stage.rexmit_psn);
    modify_field(to_s4_bktrack_to_stage_scr.pad, to_s4_bktrack_to_stage.pad);

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
    modify_field(t0_s2s_sq_bktrack_info_scr.pad, t0_s2s_sq_bktrack_info.pad);

}

action req_tx_bktrack_sqsge_process_s5 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s5_bktrack_to_stage_scr.wqe_addr, to_s5_bktrack_to_stage.wqe_addr);
    modify_field(to_s5_bktrack_to_stage_scr.log_pmtu, to_s5_bktrack_to_stage.log_pmtu);
    modify_field(to_s5_bktrack_to_stage_scr.log_sq_page_size, to_s5_bktrack_to_stage.log_sq_page_size);
    modify_field(to_s5_bktrack_to_stage_scr.log_wqe_size, to_s5_bktrack_to_stage.log_wqe_size);
    modify_field(to_s5_bktrack_to_stage_scr.log_num_wqes, to_s5_bktrack_to_stage.log_num_wqes);
    modify_field(to_s5_bktrack_to_stage_scr.rexmit_psn, to_s5_bktrack_to_stage.rexmit_psn);
    modify_field(to_s5_bktrack_to_stage_scr.pad, to_s5_bktrack_to_stage.pad);

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
    modify_field(t0_s2s_sq_bktrack_info_scr.pad, t0_s2s_sq_bktrack_info.pad);

}

action req_tx_bktrack_sqsge_process_s6 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s6_bktrack_to_stage_scr.wqe_addr, to_s6_bktrack_to_stage.wqe_addr);
    modify_field(to_s6_bktrack_to_stage_scr.log_pmtu, to_s6_bktrack_to_stage.log_pmtu);
    modify_field(to_s6_bktrack_to_stage_scr.log_sq_page_size, to_s6_bktrack_to_stage.log_sq_page_size);
    modify_field(to_s6_bktrack_to_stage_scr.log_wqe_size, to_s6_bktrack_to_stage.log_wqe_size);
    modify_field(to_s6_bktrack_to_stage_scr.log_num_wqes, to_s6_bktrack_to_stage.log_num_wqes);
    modify_field(to_s6_bktrack_to_stage_scr.rexmit_psn, to_s6_bktrack_to_stage.rexmit_psn);
    modify_field(to_s6_bktrack_to_stage_scr.pad, to_s6_bktrack_to_stage.pad);

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
    modify_field(t0_s2s_sq_bktrack_info_scr.pad, t0_s2s_sq_bktrack_info.pad);

}
action req_tx_bktrack_sqwqe_process_s2 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s2_bktrack_to_stage_scr.wqe_addr, to_s2_bktrack_to_stage.wqe_addr);
    modify_field(to_s2_bktrack_to_stage_scr.log_pmtu, to_s2_bktrack_to_stage.log_pmtu);
    modify_field(to_s2_bktrack_to_stage_scr.log_sq_page_size, to_s2_bktrack_to_stage.log_sq_page_size);
    modify_field(to_s2_bktrack_to_stage_scr.log_wqe_size, to_s2_bktrack_to_stage.log_wqe_size);
    modify_field(to_s2_bktrack_to_stage_scr.log_num_wqes, to_s2_bktrack_to_stage.log_num_wqes);
    modify_field(to_s2_bktrack_to_stage_scr.rexmit_psn, to_s2_bktrack_to_stage.rexmit_psn);
    modify_field(to_s2_bktrack_to_stage_scr.pad, to_s2_bktrack_to_stage.pad);

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
    modify_field(t0_s2s_sq_bktrack_info_scr.pad, t0_s2s_sq_bktrack_info.pad);

}

action req_tx_bktrack_sqwqe_process_s3 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s3_bktrack_to_stage_scr.wqe_addr, to_s3_bktrack_to_stage.wqe_addr);
    modify_field(to_s3_bktrack_to_stage_scr.log_pmtu, to_s3_bktrack_to_stage.log_pmtu);
    modify_field(to_s3_bktrack_to_stage_scr.log_sq_page_size, to_s3_bktrack_to_stage.log_sq_page_size);
    modify_field(to_s3_bktrack_to_stage_scr.log_wqe_size, to_s3_bktrack_to_stage.log_wqe_size);
    modify_field(to_s3_bktrack_to_stage_scr.log_num_wqes, to_s3_bktrack_to_stage.log_num_wqes);
    modify_field(to_s3_bktrack_to_stage_scr.rexmit_psn, to_s3_bktrack_to_stage.rexmit_psn);
    modify_field(to_s3_bktrack_to_stage_scr.pad, to_s3_bktrack_to_stage.pad);

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
    modify_field(t0_s2s_sq_bktrack_info_scr.pad, t0_s2s_sq_bktrack_info.pad);
}

action req_tx_bktrack_sqwqe_process_s4 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_bktrack_to_stage_scr.wqe_addr, to_s4_bktrack_to_stage.wqe_addr);
    modify_field(to_s4_bktrack_to_stage_scr.log_pmtu, to_s4_bktrack_to_stage.log_pmtu);
    modify_field(to_s4_bktrack_to_stage_scr.log_sq_page_size, to_s4_bktrack_to_stage.log_sq_page_size);
    modify_field(to_s4_bktrack_to_stage_scr.log_wqe_size, to_s4_bktrack_to_stage.log_wqe_size);
    modify_field(to_s4_bktrack_to_stage_scr.log_num_wqes, to_s4_bktrack_to_stage.log_num_wqes);
    modify_field(to_s4_bktrack_to_stage_scr.rexmit_psn, to_s4_bktrack_to_stage.rexmit_psn);
    modify_field(to_s4_bktrack_to_stage_scr.pad, to_s4_bktrack_to_stage.pad);

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
    modify_field(t0_s2s_sq_bktrack_info_scr.pad, t0_s2s_sq_bktrack_info.pad);
}


action req_tx_bktrack_sqwqe_process_s5 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s5_bktrack_to_stage_scr.wqe_addr, to_s5_bktrack_to_stage.wqe_addr);
    modify_field(to_s5_bktrack_to_stage_scr.log_pmtu, to_s5_bktrack_to_stage.log_pmtu);
    modify_field(to_s5_bktrack_to_stage_scr.log_sq_page_size, to_s5_bktrack_to_stage.log_sq_page_size);
    modify_field(to_s5_bktrack_to_stage_scr.log_wqe_size, to_s5_bktrack_to_stage.log_wqe_size);
    modify_field(to_s5_bktrack_to_stage_scr.log_num_wqes, to_s5_bktrack_to_stage.log_num_wqes);
    modify_field(to_s5_bktrack_to_stage_scr.rexmit_psn, to_s5_bktrack_to_stage.rexmit_psn);
    modify_field(to_s5_bktrack_to_stage_scr.pad, to_s5_bktrack_to_stage.pad);

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
    modify_field(t0_s2s_sq_bktrack_info_scr.pad, t0_s2s_sq_bktrack_info.pad);
}


action req_tx_bktrack_sqwqe_process_s6 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s6_bktrack_to_stage_scr.wqe_addr, to_s6_bktrack_to_stage.wqe_addr);
    modify_field(to_s6_bktrack_to_stage_scr.log_pmtu, to_s6_bktrack_to_stage.log_pmtu);
    modify_field(to_s6_bktrack_to_stage_scr.log_sq_page_size, to_s6_bktrack_to_stage.log_sq_page_size);
    modify_field(to_s6_bktrack_to_stage_scr.log_wqe_size, to_s6_bktrack_to_stage.log_wqe_size);
    modify_field(to_s6_bktrack_to_stage_scr.log_num_wqes, to_s6_bktrack_to_stage.log_num_wqes);
    modify_field(to_s6_bktrack_to_stage_scr.rexmit_psn, to_s6_bktrack_to_stage.rexmit_psn);
    modify_field(to_s6_bktrack_to_stage_scr.pad, to_s6_bktrack_to_stage.pad);

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
    modify_field(t0_s2s_sq_bktrack_info_scr.pad, t0_s2s_sq_bktrack_info.pad);
}

action req_tx_bktrack_write_back_process_s2 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s2_bktrack_to_stage_scr.wqe_addr, to_s2_bktrack_to_stage.wqe_addr);
    modify_field(to_s2_bktrack_to_stage_scr.log_pmtu, to_s2_bktrack_to_stage.log_pmtu);
    modify_field(to_s2_bktrack_to_stage_scr.log_sq_page_size, to_s2_bktrack_to_stage.log_sq_page_size);
    modify_field(to_s2_bktrack_to_stage_scr.log_wqe_size, to_s2_bktrack_to_stage.log_wqe_size);
    modify_field(to_s2_bktrack_to_stage_scr.log_num_wqes, to_s2_bktrack_to_stage.log_num_wqes);
    modify_field(to_s2_bktrack_to_stage_scr.rexmit_psn, to_s2_bktrack_to_stage.rexmit_psn);
    modify_field(to_s2_bktrack_to_stage_scr.pad, to_s2_bktrack_to_stage.pad);

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
    modify_field(t0_s2s_sqcb_write_back_info_scr.rsvd, t0_s2s_sqcb_write_back_info.rsvd);
    modify_field(t0_s2s_sqcb_write_back_info_scr.op_info, t0_s2s_sqcb_write_back_info.op_info);

}

action req_tx_bktrack_write_back_process_s3 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s3_bktrack_to_stage_scr.wqe_addr, to_s3_bktrack_to_stage.wqe_addr);
    modify_field(to_s3_bktrack_to_stage_scr.log_pmtu, to_s3_bktrack_to_stage.log_pmtu);
    modify_field(to_s3_bktrack_to_stage_scr.log_sq_page_size, to_s3_bktrack_to_stage.log_sq_page_size);
    modify_field(to_s3_bktrack_to_stage_scr.log_wqe_size, to_s3_bktrack_to_stage.log_wqe_size);
    modify_field(to_s3_bktrack_to_stage_scr.log_num_wqes, to_s3_bktrack_to_stage.log_num_wqes);
    modify_field(to_s3_bktrack_to_stage_scr.rexmit_psn, to_s3_bktrack_to_stage.rexmit_psn);
    modify_field(to_s3_bktrack_to_stage_scr.pad, to_s3_bktrack_to_stage.pad);

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
    modify_field(t0_s2s_sqcb_write_back_info_scr.rsvd, t0_s2s_sqcb_write_back_info.rsvd);
    modify_field(t0_s2s_sqcb_write_back_info_scr.op_info, t0_s2s_sqcb_write_back_info.op_info);

}
action req_tx_bktrack_write_back_process_s4 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_bktrack_to_stage_scr.wqe_addr, to_s4_bktrack_to_stage.wqe_addr);
    modify_field(to_s4_bktrack_to_stage_scr.log_pmtu, to_s4_bktrack_to_stage.log_pmtu);
    modify_field(to_s4_bktrack_to_stage_scr.log_sq_page_size, to_s4_bktrack_to_stage.log_sq_page_size);
    modify_field(to_s4_bktrack_to_stage_scr.log_wqe_size, to_s4_bktrack_to_stage.log_wqe_size);
    modify_field(to_s4_bktrack_to_stage_scr.log_num_wqes, to_s4_bktrack_to_stage.log_num_wqes);
    modify_field(to_s4_bktrack_to_stage_scr.rexmit_psn, to_s4_bktrack_to_stage.rexmit_psn);
    modify_field(to_s4_bktrack_to_stage_scr.pad, to_s4_bktrack_to_stage.pad);

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
    modify_field(t0_s2s_sqcb_write_back_info_scr.rsvd, t0_s2s_sqcb_write_back_info.rsvd);
    modify_field(t0_s2s_sqcb_write_back_info_scr.op_info, t0_s2s_sqcb_write_back_info.op_info);

}
action req_tx_bktrack_write_back_process_s5 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s5_bktrack_to_stage_scr.wqe_addr, to_s5_bktrack_to_stage.wqe_addr);
    modify_field(to_s5_bktrack_to_stage_scr.log_pmtu, to_s5_bktrack_to_stage.log_pmtu);
    modify_field(to_s5_bktrack_to_stage_scr.log_sq_page_size, to_s5_bktrack_to_stage.log_sq_page_size);
    modify_field(to_s5_bktrack_to_stage_scr.log_wqe_size, to_s5_bktrack_to_stage.log_wqe_size);
    modify_field(to_s5_bktrack_to_stage_scr.log_num_wqes, to_s5_bktrack_to_stage.log_num_wqes);
    modify_field(to_s5_bktrack_to_stage_scr.rexmit_psn, to_s5_bktrack_to_stage.rexmit_psn);
    modify_field(to_s5_bktrack_to_stage_scr.pad, to_s5_bktrack_to_stage.pad);

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
    modify_field(t0_s2s_sqcb_write_back_info_scr.rsvd, t0_s2s_sqcb_write_back_info.rsvd);
    modify_field(t0_s2s_sqcb_write_back_info_scr.op_info, t0_s2s_sqcb_write_back_info.op_info);

}

action req_tx_bktrack_write_back_process_s6 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s6_bktrack_to_stage_scr.wqe_addr, to_s6_bktrack_to_stage.wqe_addr);
    modify_field(to_s6_bktrack_to_stage_scr.log_pmtu, to_s6_bktrack_to_stage.log_pmtu);
    modify_field(to_s6_bktrack_to_stage_scr.log_sq_page_size, to_s6_bktrack_to_stage.log_sq_page_size);
    modify_field(to_s6_bktrack_to_stage_scr.log_wqe_size, to_s6_bktrack_to_stage.log_wqe_size);
    modify_field(to_s6_bktrack_to_stage_scr.log_num_wqes, to_s6_bktrack_to_stage.log_num_wqes);
    modify_field(to_s6_bktrack_to_stage_scr.rexmit_psn, to_s6_bktrack_to_stage.rexmit_psn);
    modify_field(to_s6_bktrack_to_stage_scr.pad, to_s6_bktrack_to_stage.pad);

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
    modify_field(t0_s2s_sqcb_write_back_info_scr.rsvd, t0_s2s_sqcb_write_back_info.rsvd);
    modify_field(t0_s2s_sqcb_write_back_info_scr.op_info, t0_s2s_sqcb_write_back_info.op_info);

}

action req_tx_bktrack_write_back_process_s7 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s7_bktrack_to_stage_scr.wqe_addr, to_s7_bktrack_to_stage.wqe_addr);
    modify_field(to_s7_bktrack_to_stage_scr.log_pmtu, to_s7_bktrack_to_stage.log_pmtu);
    modify_field(to_s7_bktrack_to_stage_scr.log_sq_page_size, to_s7_bktrack_to_stage.log_sq_page_size);
    modify_field(to_s7_bktrack_to_stage_scr.log_wqe_size, to_s7_bktrack_to_stage.log_wqe_size);
    modify_field(to_s7_bktrack_to_stage_scr.log_num_wqes, to_s7_bktrack_to_stage.log_num_wqes);
    modify_field(to_s7_bktrack_to_stage_scr.rexmit_psn, to_s7_bktrack_to_stage.rexmit_psn);
    modify_field(to_s7_bktrack_to_stage_scr.pad, to_s7_bktrack_to_stage.pad);

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
    modify_field(t0_s2s_sqcb_write_back_info_scr.rsvd, t0_s2s_sqcb_write_back_info.rsvd);
    modify_field(t0_s2s_sqcb_write_back_info_scr.op_info, t0_s2s_sqcb_write_back_info.op_info);

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
    modify_field(t1_s2s_sge_to_lkey_info_scr.pad, t1_s2s_sge_to_lkey_info.pad);

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
    modify_field(t0_s2s_lkey_to_ptseg_info_scr.pad, t0_s2s_lkey_to_ptseg_info.pad);

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
    modify_field(t1_s2s_lkey_to_ptseg_info_scr.pad, t1_s2s_lkey_to_ptseg_info.pad);

}

action req_tx_add_headers_2_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

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
    modify_field(t0_s2s_sqcb1_to_credits_info_scr.need_credits, t0_s2s_sqcb1_to_credits_info.need_credits);
    modify_field(t0_s2s_sqcb1_to_credits_info_scr.pad, t0_s2s_sqcb1_to_credits_info.pad);

}

action req_tx_sqsge_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s3_sq_to_stage_scr.wqe_addr, to_s3_sq_to_stage.wqe_addr);
    modify_field(to_s3_sq_to_stage_scr.spec_cindex, to_s3_sq_to_stage.spec_cindex);
    modify_field(to_s3_sq_to_stage_scr.header_template_addr, to_s3_sq_to_stage.header_template_addr);
    modify_field(to_s3_sq_to_stage_scr.packet_len, to_s3_sq_to_stage.packet_len);
    modify_field(to_s3_sq_to_stage_scr.congestion_mgmt_enable, to_s3_sq_to_stage.congestion_mgmt_enable);
    modify_field(to_s3_sq_to_stage_scr.rate_enforce_failed, to_s3_sq_to_stage.rate_enforce_failed);

    // stage to stage
    modify_field(t0_s2s_wqe_to_sge_info_scr.in_progress, t0_s2s_wqe_to_sge_info.in_progress);
    modify_field(t0_s2s_wqe_to_sge_info_scr.op_type, t0_s2s_wqe_to_sge_info.op_type);
    modify_field(t0_s2s_wqe_to_sge_info_scr.first, t0_s2s_wqe_to_sge_info.first);
    modify_field(t0_s2s_wqe_to_sge_info_scr.current_sge_id, t0_s2s_wqe_to_sge_info.current_sge_id);
    modify_field(t0_s2s_wqe_to_sge_info_scr.num_valid_sges, t0_s2s_wqe_to_sge_info.num_valid_sges);
    modify_field(t0_s2s_wqe_to_sge_info_scr.current_sge_offset, t0_s2s_wqe_to_sge_info.current_sge_offset);
    modify_field(t0_s2s_wqe_to_sge_info_scr.remaining_payload_bytes, t0_s2s_wqe_to_sge_info.remaining_payload_bytes);
    modify_field(t0_s2s_wqe_to_sge_info_scr.ah_size, t0_s2s_wqe_to_sge_info.ah_size);
    modify_field(t0_s2s_wqe_to_sge_info_scr.poll_in_progress, t0_s2s_wqe_to_sge_info.poll_in_progress);
    modify_field(t0_s2s_wqe_to_sge_info_scr.color, t0_s2s_wqe_to_sge_info.color);
    modify_field(t0_s2s_wqe_to_sge_info_scr.rsvd, t0_s2s_wqe_to_sge_info.rsvd);
    modify_field(t0_s2s_wqe_to_sge_info_scr.dma_cmd_start_index, t0_s2s_wqe_to_sge_info.dma_cmd_start_index);
    modify_field(t0_s2s_wqe_to_sge_info_scr.imm_data, t0_s2s_wqe_to_sge_info.imm_data);
    modify_field(t0_s2s_wqe_to_sge_info_scr.inv_key_or_ah_handle, t0_s2s_wqe_to_sge_info.inv_key_or_ah_handle);

}

action req_tx_sqsge_process_recirc () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s0_sq_to_stage_scr.wqe_addr, to_s0_sq_to_stage.wqe_addr);
    modify_field(to_s0_sq_to_stage_scr.spec_cindex, to_s0_sq_to_stage.spec_cindex);
    modify_field(to_s0_sq_to_stage_scr.header_template_addr, to_s0_sq_to_stage.header_template_addr);
    modify_field(to_s0_sq_to_stage_scr.packet_len, to_s0_sq_to_stage.packet_len);
    modify_field(to_s0_sq_to_stage_scr.congestion_mgmt_enable, to_s0_sq_to_stage.congestion_mgmt_enable);
    modify_field(to_s0_sq_to_stage_scr.rate_enforce_failed, to_s0_sq_to_stage.rate_enforce_failed);

    // stage to stage
    modify_field(t2_s2s_wqe_to_sge_info_scr.in_progress, t2_s2s_wqe_to_sge_info.in_progress);
    modify_field(t2_s2s_wqe_to_sge_info_scr.op_type, t2_s2s_wqe_to_sge_info.op_type);
    modify_field(t2_s2s_wqe_to_sge_info_scr.first, t2_s2s_wqe_to_sge_info.first);
    modify_field(t2_s2s_wqe_to_sge_info_scr.current_sge_id, t2_s2s_wqe_to_sge_info.current_sge_id);
    modify_field(t2_s2s_wqe_to_sge_info_scr.num_valid_sges, t2_s2s_wqe_to_sge_info.num_valid_sges);
    modify_field(t2_s2s_wqe_to_sge_info_scr.current_sge_offset, t2_s2s_wqe_to_sge_info.current_sge_offset);
    modify_field(t2_s2s_wqe_to_sge_info_scr.remaining_payload_bytes, t2_s2s_wqe_to_sge_info.remaining_payload_bytes);
    modify_field(t2_s2s_wqe_to_sge_info_scr.ah_size, t2_s2s_wqe_to_sge_info.ah_size);
    modify_field(t2_s2s_wqe_to_sge_info_scr.poll_in_progress, t2_s2s_wqe_to_sge_info.poll_in_progress);
    modify_field(t2_s2s_wqe_to_sge_info_scr.color, t2_s2s_wqe_to_sge_info.color);
    modify_field(t2_s2s_wqe_to_sge_info_scr.rsvd, t2_s2s_wqe_to_sge_info.rsvd);
    modify_field(t2_s2s_wqe_to_sge_info_scr.dma_cmd_start_index, t2_s2s_wqe_to_sge_info.dma_cmd_start_index);
    modify_field(t2_s2s_wqe_to_sge_info_scr.imm_data, t2_s2s_wqe_to_sge_info.imm_data);
    modify_field(t2_s2s_wqe_to_sge_info_scr.inv_key_or_ah_handle, t2_s2s_wqe_to_sge_info.inv_key_or_ah_handle);

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
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.pad, t0_s2s_sqcb_to_wqe_info.pad);

}

action req_tx_add_headers_process_rd () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s5_sq_to_stage_scr.wqe_addr, to_s5_sq_to_stage.wqe_addr);
    modify_field(to_s5_sq_to_stage_scr.spec_cindex, to_s5_sq_to_stage.spec_cindex);
    modify_field(to_s5_sq_to_stage_scr.header_template_addr, to_s5_sq_to_stage.header_template_addr);
    modify_field(to_s5_sq_to_stage_scr.packet_len, to_s5_sq_to_stage.packet_len);
    modify_field(to_s5_sq_to_stage_scr.congestion_mgmt_enable, to_s5_sq_to_stage.congestion_mgmt_enable);
    modify_field(to_s5_sq_to_stage_scr.rate_enforce_failed, to_s5_sq_to_stage.rate_enforce_failed);

    // stage to stage
    modify_field(t3_s2s_sqcb_write_back_info_rd_scr.hdr_template_inline, t3_s2s_sqcb_write_back_info_rd.hdr_template_inline);
    modify_field(t3_s2s_sqcb_write_back_info_rd_scr.busy, t3_s2s_sqcb_write_back_info_rd.busy);
    modify_field(t3_s2s_sqcb_write_back_info_rd_scr.in_progress, t3_s2s_sqcb_write_back_info_rd.in_progress);
    modify_field(t3_s2s_sqcb_write_back_info_rd_scr.bktrack_in_progress, t3_s2s_sqcb_write_back_info_rd.bktrack_in_progress);
    modify_field(t3_s2s_sqcb_write_back_info_rd_scr.op_type, t3_s2s_sqcb_write_back_info_rd.op_type);
    modify_field(t3_s2s_sqcb_write_back_info_rd_scr.first, t3_s2s_sqcb_write_back_info_rd.first);
    modify_field(t3_s2s_sqcb_write_back_info_rd_scr.last_pkt, t3_s2s_sqcb_write_back_info_rd.last_pkt);
    modify_field(t3_s2s_sqcb_write_back_info_rd_scr.set_fence, t3_s2s_sqcb_write_back_info_rd.set_fence);
    modify_field(t3_s2s_sqcb_write_back_info_rd_scr.set_li_fence, t3_s2s_sqcb_write_back_info_rd.set_li_fence);
    modify_field(t3_s2s_sqcb_write_back_info_rd_scr.empty_rrq_bktrack, t3_s2s_sqcb_write_back_info_rd.empty_rrq_bktrack);
    modify_field(t3_s2s_sqcb_write_back_info_rd_scr.current_sge_offset, t3_s2s_sqcb_write_back_info_rd.current_sge_offset);
    modify_field(t3_s2s_sqcb_write_back_info_rd_scr.current_sge_id, t3_s2s_sqcb_write_back_info_rd.current_sge_id);
    modify_field(t3_s2s_sqcb_write_back_info_rd_scr.num_sges, t3_s2s_sqcb_write_back_info_rd.num_sges);
    modify_field(t3_s2s_sqcb_write_back_info_rd_scr.sq_c_index, t3_s2s_sqcb_write_back_info_rd.sq_c_index);
    modify_field(t3_s2s_sqcb_write_back_info_rd_scr.ah_size, t3_s2s_sqcb_write_back_info_rd.ah_size);
    modify_field(t3_s2s_sqcb_write_back_info_rd_scr.poll_in_progress, t3_s2s_sqcb_write_back_info_rd.poll_in_progress);
    modify_field(t3_s2s_sqcb_write_back_info_rd_scr.color, t3_s2s_sqcb_write_back_info_rd.color);
    modify_field(t3_s2s_sqcb_write_back_info_rd_scr.poll_failed, t3_s2s_sqcb_write_back_info_rd.poll_failed);
    modify_field(t3_s2s_sqcb_write_back_info_rd_scr.rsvd, t3_s2s_sqcb_write_back_info_rd.rsvd);
    modify_field(t3_s2s_sqcb_write_back_info_rd_scr.op_rd_read_len, t3_s2s_sqcb_write_back_info_rd.op_rd_read_len);
    modify_field(t3_s2s_sqcb_write_back_info_rd_scr.op_rd_log_pmtu, t3_s2s_sqcb_write_back_info_rd.op_rd_log_pmtu);
    modify_field(t3_s2s_sqcb_write_back_info_rd_scr.op_rd_pad, t3_s2s_sqcb_write_back_info_rd.op_rd_pad);
}

action req_tx_add_headers_process_send_wr () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s5_sq_to_stage_scr.wqe_addr, to_s5_sq_to_stage.wqe_addr);
    modify_field(to_s5_sq_to_stage_scr.spec_cindex, to_s5_sq_to_stage.spec_cindex);
    modify_field(to_s5_sq_to_stage_scr.header_template_addr, to_s5_sq_to_stage.header_template_addr);
    modify_field(to_s5_sq_to_stage_scr.packet_len, to_s5_sq_to_stage.packet_len);
    modify_field(to_s5_sq_to_stage_scr.congestion_mgmt_enable, to_s5_sq_to_stage.congestion_mgmt_enable);
    modify_field(to_s5_sq_to_stage_scr.rate_enforce_failed, to_s5_sq_to_stage.rate_enforce_failed);

    // stage to stage
    modify_field(t3_s2s_sqcb_write_back_info_send_wr_scr.hdr_template_inline, t3_s2s_sqcb_write_back_info_send_wr.hdr_template_inline);
    modify_field(t3_s2s_sqcb_write_back_info_send_wr_scr.busy, t3_s2s_sqcb_write_back_info_send_wr.busy);
    modify_field(t3_s2s_sqcb_write_back_info_send_wr_scr.in_progress, t3_s2s_sqcb_write_back_info_send_wr.in_progress);
    modify_field(t3_s2s_sqcb_write_back_info_send_wr_scr.bktrack_in_progress, t3_s2s_sqcb_write_back_info_send_wr.bktrack_in_progress);
    modify_field(t3_s2s_sqcb_write_back_info_send_wr_scr.op_type, t3_s2s_sqcb_write_back_info_send_wr.op_type);
    modify_field(t3_s2s_sqcb_write_back_info_send_wr_scr.first, t3_s2s_sqcb_write_back_info_send_wr.first);
    modify_field(t3_s2s_sqcb_write_back_info_send_wr_scr.last_pkt, t3_s2s_sqcb_write_back_info_send_wr.last_pkt);
    modify_field(t3_s2s_sqcb_write_back_info_send_wr_scr.set_fence, t3_s2s_sqcb_write_back_info_send_wr.set_fence);
    modify_field(t3_s2s_sqcb_write_back_info_send_wr_scr.set_li_fence, t3_s2s_sqcb_write_back_info_send_wr.set_li_fence);
    modify_field(t3_s2s_sqcb_write_back_info_send_wr_scr.empty_rrq_bktrack, t3_s2s_sqcb_write_back_info_send_wr.empty_rrq_bktrack);
    modify_field(t3_s2s_sqcb_write_back_info_send_wr_scr.current_sge_offset, t3_s2s_sqcb_write_back_info_send_wr.current_sge_offset);
    modify_field(t3_s2s_sqcb_write_back_info_send_wr_scr.current_sge_id, t3_s2s_sqcb_write_back_info_send_wr.current_sge_id);
    modify_field(t3_s2s_sqcb_write_back_info_send_wr_scr.num_sges, t3_s2s_sqcb_write_back_info_send_wr.num_sges);
    modify_field(t3_s2s_sqcb_write_back_info_send_wr_scr.sq_c_index, t3_s2s_sqcb_write_back_info_send_wr.sq_c_index);
    modify_field(t3_s2s_sqcb_write_back_info_send_wr_scr.ah_size, t3_s2s_sqcb_write_back_info_send_wr.ah_size);
    modify_field(t3_s2s_sqcb_write_back_info_send_wr_scr.poll_in_progress, t3_s2s_sqcb_write_back_info_send_wr.poll_in_progress);
    modify_field(t3_s2s_sqcb_write_back_info_send_wr_scr.color, t3_s2s_sqcb_write_back_info_send_wr.color);
    modify_field(t3_s2s_sqcb_write_back_info_send_wr_scr.poll_failed, t3_s2s_sqcb_write_back_info_send_wr.poll_failed);
    modify_field(t3_s2s_sqcb_write_back_info_send_wr_scr.rsvd, t3_s2s_sqcb_write_back_info_send_wr.rsvd);
    modify_field(t3_s2s_sqcb_write_back_info_send_wr_scr.op_send_wr_imm_data, t3_s2s_sqcb_write_back_info_send_wr.op_send_wr_imm_data);
    modify_field(t3_s2s_sqcb_write_back_info_send_wr_scr.op_send_wr_inv_key_or_ah_handle, t3_s2s_sqcb_write_back_info_send_wr.op_send_wr_inv_key_or_ah_handle);
}


action req_tx_dcqcn_enforce_process_s3 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s3_sq_to_stage_scr.wqe_addr, to_s3_sq_to_stage.wqe_addr);
    modify_field(to_s3_sq_to_stage_scr.spec_cindex, to_s3_sq_to_stage.spec_cindex);
    modify_field(to_s3_sq_to_stage_scr.header_template_addr, to_s3_sq_to_stage.header_template_addr);
    modify_field(to_s3_sq_to_stage_scr.packet_len, to_s3_sq_to_stage.packet_len);
    modify_field(to_s3_sq_to_stage_scr.congestion_mgmt_enable, to_s3_sq_to_stage.congestion_mgmt_enable);
    modify_field(to_s3_sq_to_stage_scr.rate_enforce_failed, to_s3_sq_to_stage.rate_enforce_failed);

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
    modify_field(t2_s2s_sqcb_write_back_info_scr.rsvd, t2_s2s_sqcb_write_back_info.rsvd);

}

action req_tx_dcqcn_enforce_process_s4 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_sq_to_stage_scr.wqe_addr, to_s4_sq_to_stage.wqe_addr);
    modify_field(to_s4_sq_to_stage_scr.spec_cindex, to_s4_sq_to_stage.spec_cindex);
    modify_field(to_s4_sq_to_stage_scr.header_template_addr, to_s4_sq_to_stage.header_template_addr);
    modify_field(to_s4_sq_to_stage_scr.packet_len, to_s4_sq_to_stage.packet_len);
    modify_field(to_s4_sq_to_stage_scr.congestion_mgmt_enable, to_s4_sq_to_stage.congestion_mgmt_enable);
    modify_field(to_s4_sq_to_stage_scr.rate_enforce_failed, to_s4_sq_to_stage.rate_enforce_failed);

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
    modify_field(to_s2_sq_to_stage_scr.wqe_addr, to_s2_sq_to_stage.wqe_addr);
    modify_field(to_s2_sq_to_stage_scr.spec_cindex, to_s2_sq_to_stage.spec_cindex);
    modify_field(to_s2_sq_to_stage_scr.header_template_addr, to_s2_sq_to_stage.header_template_addr);
    modify_field(to_s2_sq_to_stage_scr.packet_len, to_s2_sq_to_stage.packet_len);
    modify_field(to_s2_sq_to_stage_scr.congestion_mgmt_enable, to_s2_sq_to_stage.congestion_mgmt_enable);
    modify_field(to_s2_sq_to_stage_scr.rate_enforce_failed, to_s2_sq_to_stage.rate_enforce_failed);

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
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.pad, t0_s2s_sqcb_to_wqe_info.pad);

}

action req_tx_write_back_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s5_sq_to_stage_scr.wqe_addr, to_s5_sq_to_stage.wqe_addr);
    modify_field(to_s5_sq_to_stage_scr.spec_cindex, to_s5_sq_to_stage.spec_cindex);
    modify_field(to_s5_sq_to_stage_scr.header_template_addr, to_s5_sq_to_stage.header_template_addr);
    modify_field(to_s5_sq_to_stage_scr.packet_len, to_s5_sq_to_stage.packet_len);
    modify_field(to_s5_sq_to_stage_scr.congestion_mgmt_enable, to_s5_sq_to_stage.congestion_mgmt_enable);
    modify_field(to_s5_sq_to_stage_scr.rate_enforce_failed, to_s5_sq_to_stage.rate_enforce_failed);

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
    modify_field(t2_s2s_sqcb_write_back_info_scr.rsvd, t2_s2s_sqcb_write_back_info.rsvd);

}

action req_tx_write_back_process_rd () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s5_sq_to_stage_scr.wqe_addr, to_s5_sq_to_stage.wqe_addr);
    modify_field(to_s5_sq_to_stage_scr.spec_cindex, to_s5_sq_to_stage.spec_cindex);
    modify_field(to_s5_sq_to_stage_scr.header_template_addr, to_s5_sq_to_stage.header_template_addr);
    modify_field(to_s5_sq_to_stage_scr.packet_len, to_s5_sq_to_stage.packet_len);
    modify_field(to_s5_sq_to_stage_scr.congestion_mgmt_enable, to_s5_sq_to_stage.congestion_mgmt_enable);
    modify_field(to_s5_sq_to_stage_scr.rate_enforce_failed, to_s5_sq_to_stage.rate_enforce_failed);

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
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.rsvd, t2_s2s_sqcb_write_back_info_rd.rsvd);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.op_rd_read_len, t2_s2s_sqcb_write_back_info_rd.op_rd_read_len);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.op_rd_log_pmtu, t2_s2s_sqcb_write_back_info_rd.op_rd_log_pmtu);
    modify_field(t2_s2s_sqcb_write_back_info_rd_scr.op_rd_pad, t2_s2s_sqcb_write_back_info_rd.op_rd_pad);
}

action req_tx_write_back_process_send_wr () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s5_sq_to_stage_scr.wqe_addr, to_s5_sq_to_stage.wqe_addr);
    modify_field(to_s5_sq_to_stage_scr.spec_cindex, to_s5_sq_to_stage.spec_cindex);
    modify_field(to_s5_sq_to_stage_scr.header_template_addr, to_s5_sq_to_stage.header_template_addr);
    modify_field(to_s5_sq_to_stage_scr.packet_len, to_s5_sq_to_stage.packet_len);
    modify_field(to_s5_sq_to_stage_scr.congestion_mgmt_enable, to_s5_sq_to_stage.congestion_mgmt_enable);
    modify_field(to_s5_sq_to_stage_scr.rate_enforce_failed, to_s5_sq_to_stage.rate_enforce_failed);

    // stage to stage
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.hdr_template_inline, t2_s2s_sqcb_write_back_info_send_wr.hdr_template_inline);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.busy, t2_s2s_sqcb_write_back_info_send_wr.busy);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.in_progress, t2_s2s_sqcb_write_back_info_send_wr.in_progress);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.bktrack_in_progress, t2_s2s_sqcb_write_back_info_send_wr.bktrack_in_progress);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.op_type, t2_s2s_sqcb_write_back_info_send_wr.op_type);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.first, t2_s2s_sqcb_write_back_info_send_wr.first);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.last_pkt, t2_s2s_sqcb_write_back_info_send_wr.last_pkt);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.set_fence, t2_s2s_sqcb_write_back_info_send_wr.set_fence);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.set_li_fence, t2_s2s_sqcb_write_back_info_send_wr.set_li_fence);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.empty_rrq_bktrack, t2_s2s_sqcb_write_back_info_send_wr.empty_rrq_bktrack);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.current_sge_offset, t2_s2s_sqcb_write_back_info_send_wr.current_sge_offset);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.current_sge_id, t2_s2s_sqcb_write_back_info_send_wr.current_sge_id);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.num_sges, t2_s2s_sqcb_write_back_info_send_wr.num_sges);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.sq_c_index, t2_s2s_sqcb_write_back_info_send_wr.sq_c_index);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.ah_size, t2_s2s_sqcb_write_back_info_send_wr.ah_size);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.poll_in_progress, t2_s2s_sqcb_write_back_info_send_wr.poll_in_progress);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.color, t2_s2s_sqcb_write_back_info_send_wr.color);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.poll_failed, t2_s2s_sqcb_write_back_info_send_wr.poll_failed);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.rsvd, t2_s2s_sqcb_write_back_info_send_wr.rsvd);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.op_send_wr_imm_data, t2_s2s_sqcb_write_back_info_send_wr.op_send_wr_imm_data);
    modify_field(t2_s2s_sqcb_write_back_info_send_wr_scr.op_send_wr_inv_key_or_ah_handle, t2_s2s_sqcb_write_back_info_send_wr.op_send_wr_inv_key_or_ah_handle);
}

action req_tx_sqsge_iterate_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_sq_to_stage_scr.wqe_addr, to_s1_sq_to_stage.wqe_addr);
    modify_field(to_s1_sq_to_stage_scr.spec_cindex, to_s1_sq_to_stage.spec_cindex);
    modify_field(to_s1_sq_to_stage_scr.header_template_addr, to_s1_sq_to_stage.header_template_addr);
    modify_field(to_s1_sq_to_stage_scr.packet_len, to_s1_sq_to_stage.packet_len);
    modify_field(to_s1_sq_to_stage_scr.congestion_mgmt_enable, to_s1_sq_to_stage.congestion_mgmt_enable);
    modify_field(to_s1_sq_to_stage_scr.rate_enforce_failed, to_s1_sq_to_stage.rate_enforce_failed);

    // stage to stage
    modify_field(t2_s2s_wqe_to_sge_info_scr.in_progress, t2_s2s_wqe_to_sge_info.in_progress);
    modify_field(t2_s2s_wqe_to_sge_info_scr.op_type, t2_s2s_wqe_to_sge_info.op_type);
    modify_field(t2_s2s_wqe_to_sge_info_scr.first, t2_s2s_wqe_to_sge_info.first);
    modify_field(t2_s2s_wqe_to_sge_info_scr.current_sge_id, t2_s2s_wqe_to_sge_info.current_sge_id);
    modify_field(t2_s2s_wqe_to_sge_info_scr.num_valid_sges, t2_s2s_wqe_to_sge_info.num_valid_sges);
    modify_field(t2_s2s_wqe_to_sge_info_scr.current_sge_offset, t2_s2s_wqe_to_sge_info.current_sge_offset);
    modify_field(t2_s2s_wqe_to_sge_info_scr.remaining_payload_bytes, t2_s2s_wqe_to_sge_info.remaining_payload_bytes);
    modify_field(t2_s2s_wqe_to_sge_info_scr.ah_size, t2_s2s_wqe_to_sge_info.ah_size);
    modify_field(t2_s2s_wqe_to_sge_info_scr.poll_in_progress, t2_s2s_wqe_to_sge_info.poll_in_progress);
    modify_field(t2_s2s_wqe_to_sge_info_scr.color, t2_s2s_wqe_to_sge_info.color);
    modify_field(t2_s2s_wqe_to_sge_info_scr.rsvd, t2_s2s_wqe_to_sge_info.rsvd);
    modify_field(t2_s2s_wqe_to_sge_info_scr.dma_cmd_start_index, t2_s2s_wqe_to_sge_info.dma_cmd_start_index);
    modify_field(t2_s2s_wqe_to_sge_info_scr.imm_data, t2_s2s_wqe_to_sge_info.imm_data);
    modify_field(t2_s2s_wqe_to_sge_info_scr.inv_key_or_ah_handle, t2_s2s_wqe_to_sge_info.inv_key_or_ah_handle);

}

