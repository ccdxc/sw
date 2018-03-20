/***********************************************************************/
/* rdma_req_txdma.p4 */
/***********************************************************************/

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

#define tx_table_s1_t0_action req_tx_sqwqe_process
#define tx_table_s1_t0_action1 req_tx_sqpt_process
#define tx_table_s1_t0_action2 req_tx_sqcb1_process
#define tx_table_s1_t0_action3 req_tx_bktrack_sqcb2_process
#define tx_table_s1_t0_action4 req_tx_sqcb2_cnp_process

#define tx_table_s1_t2_action req_tx_sqsge_iterate_process

#define tx_table_s2_t0_action req_tx_sqsge_process
#define tx_table_s2_t0_action1 req_tx_credits_process
#define tx_table_s2_t0_action2 req_tx_bktrack_sqwqe_process
#define tx_table_s2_t0_action3 req_tx_bktrack_sqpt_process
#define tx_table_s2_t0_action4 req_tx_bktrack_write_back_process
#define tx_table_s2_t0_action5 req_tx_dcqcn_cnp_process

#define tx_table_s2_t2_action req_tx_dcqcn_enforce_process

#define tx_table_s3_t0_action req_tx_sqlkey_process
#define tx_table_s3_t0_action1 req_tx_bktrack_sqsge_process

#define tx_table_s3_t1_action req_tx_bktrack_sqcb2_write_back_process

#define tx_table_s3_t2_action req_tx_add_headers_process

#define tx_table_s3_t3_action req_tx_load_hdr_template_process

#define tx_table_s4_t0_action req_tx_sqptseg_process

#define tx_table_s4_t2_action req_tx_add_headers_2_process

#include "../common-p4+/common_txdma.p4"

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
    modify_field(phv_global_common_scr.incr_lsn, phv_global_common.incr_lsn);\
    modify_field(phv_global_common_scr.immeth_vld, phv_global_common.immeth_vld);\

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
        pad                              :   22;
        error_disable_qp                 :    1;
        incr_lsn                         :    1;
        immeth_vld                       :    1;
    }
}

header_type req_tx_bktrack_sqcb2_write_back_info_t {
    fields {
        wqe_start_psn                    :   24;
        tx_psn                           :   24;
        ssn                              :   24;
        skip_wqe_start_psn               :    1;
        tbl_id                           :    3;
        imm_data                         :   32;
        inv_key                          :   32;
        op_type                          :    8;
        pad                              :   12;
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
        last                             :    1;
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
        op_type                          :    5;
        sq_in_hbm                        :    1;
        pad                              :   38;
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
        op_type                          :    8;
        first                            :    1;
        last                             :    1;
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
        rsvd                             :    5;
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

/**** global header unions ****/

@pragma pa_header_union ingress common_global
metadata phv_global_common_t phv_global_common;
@pragma scratch_metadata
metadata phv_global_common_t phv_global_common_scr;

/**** to stage header unions ****/

@pragma pa_header_union ingress to_stage_1
metadata req_tx_sq_to_stage_t to_s1_sq_to_stage;
@pragma scratch_metadata
metadata req_tx_sq_to_stage_t to_s1_sq_to_stage_scr;

@pragma pa_header_union ingress to_stage_1
metadata req_tx_bktrack_to_stage_t to_s1_bktrack_to_stage;
@pragma scratch_metadata
metadata req_tx_bktrack_to_stage_t to_s1_bktrack_to_stage_scr;

@pragma pa_header_union ingress to_stage_2
metadata req_tx_sq_to_stage_t to_s2_sq_to_stage;
@pragma scratch_metadata
metadata req_tx_sq_to_stage_t to_s2_sq_to_stage_scr;

@pragma pa_header_union ingress to_stage_2
metadata req_tx_bktrack_to_stage_t to_s2_bktrack_to_stage;
@pragma scratch_metadata
metadata req_tx_bktrack_to_stage_t to_s2_bktrack_to_stage_scr;

@pragma pa_header_union ingress to_stage_3
metadata req_tx_bktrack_to_stage_t to_s3_bktrack_to_stage;
@pragma scratch_metadata
metadata req_tx_bktrack_to_stage_t to_s3_bktrack_to_stage_scr;

@pragma pa_header_union ingress to_stage_3
metadata req_tx_sq_to_stage_t to_s3_sq_to_stage;
@pragma scratch_metadata
metadata req_tx_sq_to_stage_t to_s3_sq_to_stage_scr;

/**** stage to stage header unions ****/

@pragma pa_header_union ingress common_t0_s2s
metadata req_tx_sqcb_to_wqe_info_t t0_s2s_sqcb_to_wqe_info;
@pragma scratch_metadata
metadata req_tx_sqcb_to_wqe_info_t t0_s2s_sqcb_to_wqe_info_scr;

@pragma pa_header_union ingress common_t0_s2s
metadata req_tx_sqcb_to_pt_info_t t0_s2s_sqcb_to_pt_info;
@pragma scratch_metadata
metadata req_tx_sqcb_to_pt_info_t t0_s2s_sqcb_to_pt_info_scr;

@pragma pa_header_union ingress common_t0_s2s
metadata req_tx_sqcb0_to_sqcb2_info_t t0_s2s_sqcb0_to_sqcb2_info;
@pragma scratch_metadata
metadata req_tx_sqcb0_to_sqcb2_info_t t0_s2s_sqcb0_to_sqcb2_info_scr;

@pragma pa_header_union ingress common_t0_s2s
metadata req_tx_wqe_to_sge_info_t t0_s2s_wqe_to_sge_info;
@pragma scratch_metadata
metadata req_tx_wqe_to_sge_info_t t0_s2s_wqe_to_sge_info_scr;

@pragma pa_header_union ingress common_t0_s2s
metadata req_tx_sqcb1_to_credits_info_t t0_s2s_sqcb1_to_credits_info;
@pragma scratch_metadata
metadata req_tx_sqcb1_to_credits_info_t t0_s2s_sqcb1_to_credits_info_scr;

@pragma pa_header_union ingress common_t0_s2s
metadata req_tx_sq_bktrack_info_t t0_s2s_sq_bktrack_info;
@pragma scratch_metadata
metadata req_tx_sq_bktrack_info_t t0_s2s_sq_bktrack_info_scr;

@pragma pa_header_union ingress common_t0_s2s
metadata req_tx_sqcb_write_back_info_t t0_s2s_sqcb_write_back_info;
@pragma scratch_metadata
metadata req_tx_sqcb_write_back_info_t t0_s2s_sqcb_write_back_info_scr;

@pragma pa_header_union ingress common_t0_s2s
metadata req_tx_sge_to_lkey_info_t t0_s2s_sge_to_lkey_info;
@pragma scratch_metadata
metadata req_tx_sge_to_lkey_info_t t0_s2s_sge_to_lkey_info_scr;

@pragma pa_header_union ingress common_t0_s2s
metadata req_tx_lkey_to_ptseg_info_t t0_s2s_lkey_to_ptseg_info;
@pragma scratch_metadata
metadata req_tx_lkey_to_ptseg_info_t t0_s2s_lkey_to_ptseg_info_scr;

@pragma pa_header_union ingress common_t1_s2s
metadata req_tx_bktrack_sqcb2_write_back_info_t t1_s2s_bktrack_sqcb2_write_back_info;
@pragma scratch_metadata
metadata req_tx_bktrack_sqcb2_write_back_info_t t1_s2s_bktrack_sqcb2_write_back_info_scr;

@pragma pa_header_union ingress common_t2_s2s
metadata req_tx_wqe_to_sge_info_t t2_s2s_wqe_to_sge_info;
@pragma scratch_metadata
metadata req_tx_wqe_to_sge_info_t t2_s2s_wqe_to_sge_info_scr;

@pragma pa_header_union ingress common_t2_s2s
metadata req_tx_add_hdr_info_t t2_s2s_add_hdr_info;
@pragma scratch_metadata
metadata req_tx_add_hdr_info_t t2_s2s_add_hdr_info_scr;

@pragma pa_header_union ingress common_t2_s2s
metadata req_tx_sqcb_write_back_info_t t2_s2s_sqcb_write_back_info;
@pragma scratch_metadata
metadata req_tx_sqcb_write_back_info_t t2_s2s_sqcb_write_back_info_scr;


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
    modify_field(t0_s2s_sq_bktrack_info_scr.op_type, t0_s2s_sq_bktrack_info.op_type);
    modify_field(t0_s2s_sq_bktrack_info_scr.sq_p_index, t0_s2s_sq_bktrack_info.sq_p_index);
    modify_field(t0_s2s_sq_bktrack_info_scr.pad1, t0_s2s_sq_bktrack_info.pad1);
    modify_field(t0_s2s_sq_bktrack_info_scr.imm_data, t0_s2s_sq_bktrack_info.imm_data);
    modify_field(t0_s2s_sq_bktrack_info_scr.inv_key, t0_s2s_sq_bktrack_info.inv_key);
    modify_field(t0_s2s_sq_bktrack_info_scr.pad2, t0_s2s_sq_bktrack_info.pad2);

}
action req_tx_dcqcn_cnp_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage

}
action req_tx_sqcb1_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

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
action req_tx_sqlkey_process () {
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
action req_tx_sqptseg_process () {
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
action req_tx_add_headers_2_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t2_s2s_add_hdr_info_scr.hdr_template_inline, t2_s2s_add_hdr_info.hdr_template_inline);
    modify_field(t2_s2s_add_hdr_info_scr.service, t2_s2s_add_hdr_info.service);
    modify_field(t2_s2s_add_hdr_info_scr.header_template_addr, t2_s2s_add_hdr_info.header_template_addr);
    modify_field(t2_s2s_add_hdr_info_scr.header_template_size, t2_s2s_add_hdr_info.header_template_size);
    modify_field(t2_s2s_add_hdr_info_scr.roce_opt_ts_enable, t2_s2s_add_hdr_info.roce_opt_ts_enable);
    modify_field(t2_s2s_add_hdr_info_scr.roce_opt_mss_enable, t2_s2s_add_hdr_info.roce_opt_mss_enable);
    modify_field(t2_s2s_add_hdr_info_scr.pad, t2_s2s_add_hdr_info.pad);

}
action req_tx_add_headers_process () {
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
    modify_field(t2_s2s_sqcb_write_back_info_scr.last, t2_s2s_sqcb_write_back_info.last);
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
    modify_field(t2_s2s_sqcb_write_back_info_scr.rsvd, t2_s2s_sqcb_write_back_info.rsvd);

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
action req_tx_bktrack_sqcb2_write_back_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t1_s2s_bktrack_sqcb2_write_back_info_scr.wqe_start_psn, t1_s2s_bktrack_sqcb2_write_back_info.wqe_start_psn);
    modify_field(t1_s2s_bktrack_sqcb2_write_back_info_scr.tx_psn, t1_s2s_bktrack_sqcb2_write_back_info.tx_psn);
    modify_field(t1_s2s_bktrack_sqcb2_write_back_info_scr.ssn, t1_s2s_bktrack_sqcb2_write_back_info.ssn);
    modify_field(t1_s2s_bktrack_sqcb2_write_back_info_scr.skip_wqe_start_psn, t1_s2s_bktrack_sqcb2_write_back_info.skip_wqe_start_psn);
    modify_field(t1_s2s_bktrack_sqcb2_write_back_info_scr.tbl_id, t1_s2s_bktrack_sqcb2_write_back_info.tbl_id);
    modify_field(t1_s2s_bktrack_sqcb2_write_back_info_scr.imm_data, t1_s2s_bktrack_sqcb2_write_back_info.imm_data);
    modify_field(t1_s2s_bktrack_sqcb2_write_back_info_scr.inv_key, t1_s2s_bktrack_sqcb2_write_back_info.inv_key);
    modify_field(t1_s2s_bktrack_sqcb2_write_back_info_scr.op_type, t1_s2s_bktrack_sqcb2_write_back_info.op_type);
    modify_field(t1_s2s_bktrack_sqcb2_write_back_info_scr.pad, t1_s2s_bktrack_sqcb2_write_back_info.pad);

}
action req_tx_bktrack_sqsge_process () {
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
    modify_field(t0_s2s_sq_bktrack_info_scr.sq_p_index, t0_s2s_sq_bktrack_info.sq_p_index);
    modify_field(t0_s2s_sq_bktrack_info_scr.pad1, t0_s2s_sq_bktrack_info.pad1);
    modify_field(t0_s2s_sq_bktrack_info_scr.imm_data, t0_s2s_sq_bktrack_info.imm_data);
    modify_field(t0_s2s_sq_bktrack_info_scr.inv_key, t0_s2s_sq_bktrack_info.inv_key);
    modify_field(t0_s2s_sq_bktrack_info_scr.pad2, t0_s2s_sq_bktrack_info.pad2);

}
action req_tx_bktrack_sqwqe_process () {
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
    modify_field(t0_s2s_sq_bktrack_info_scr.sq_p_index, t0_s2s_sq_bktrack_info.sq_p_index);
    modify_field(t0_s2s_sq_bktrack_info_scr.pad1, t0_s2s_sq_bktrack_info.pad1);
    modify_field(t0_s2s_sq_bktrack_info_scr.imm_data, t0_s2s_sq_bktrack_info.imm_data);
    modify_field(t0_s2s_sq_bktrack_info_scr.inv_key, t0_s2s_sq_bktrack_info.inv_key);
    modify_field(t0_s2s_sq_bktrack_info_scr.pad2, t0_s2s_sq_bktrack_info.pad2);

}
action req_tx_bktrack_write_back_process () {
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
    modify_field(t0_s2s_sqcb_write_back_info_scr.last, t0_s2s_sqcb_write_back_info.last);
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
    modify_field(t0_s2s_sqcb_write_back_info_scr.rsvd, t0_s2s_sqcb_write_back_info.rsvd);

}
action req_tx_credits_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t0_s2s_sqcb1_to_credits_info_scr.need_credits, t0_s2s_sqcb1_to_credits_info.need_credits);
    modify_field(t0_s2s_sqcb1_to_credits_info_scr.pad, t0_s2s_sqcb1_to_credits_info.pad);

}
action req_tx_dcqcn_enforce_process () {
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
    modify_field(t2_s2s_add_hdr_info_scr.hdr_template_inline, t2_s2s_add_hdr_info.hdr_template_inline);
    modify_field(t2_s2s_add_hdr_info_scr.service, t2_s2s_add_hdr_info.service);
    modify_field(t2_s2s_add_hdr_info_scr.header_template_addr, t2_s2s_add_hdr_info.header_template_addr);
    modify_field(t2_s2s_add_hdr_info_scr.header_template_size, t2_s2s_add_hdr_info.header_template_size);
    modify_field(t2_s2s_add_hdr_info_scr.roce_opt_ts_enable, t2_s2s_add_hdr_info.roce_opt_ts_enable);
    modify_field(t2_s2s_add_hdr_info_scr.roce_opt_mss_enable, t2_s2s_add_hdr_info.roce_opt_mss_enable);
    modify_field(t2_s2s_add_hdr_info_scr.pad, t2_s2s_add_hdr_info.pad);

}
action req_tx_load_hdr_template_process () {
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
action req_tx_sqcb_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage

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
    modify_field(t2_s2s_wqe_to_sge_info_scr.inv_key, t2_s2s_wqe_to_sge_info.inv_key);
    modify_field(t2_s2s_wqe_to_sge_info_scr.ah_handle, t2_s2s_wqe_to_sge_info.ah_handle);

}
action req_tx_sqsge_process () {
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
    modify_field(t0_s2s_wqe_to_sge_info_scr.inv_key, t0_s2s_wqe_to_sge_info.inv_key);
    modify_field(t0_s2s_wqe_to_sge_info_scr.ah_handle, t0_s2s_wqe_to_sge_info.ah_handle);

}
action req_tx_sqwqe_process () {
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

