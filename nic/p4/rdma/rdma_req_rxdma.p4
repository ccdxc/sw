/***********************************************************************/
/* rdma_req_rxdma.p4 */
/***********************************************************************/

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

#define rx_table_s1_t2_action req_rx_dcqcn_ecn_process

#define rx_table_s1_t3_action req_rx_cqcb_process

#define rx_table_s2_t0_action req_rx_rrqsge_process
#define rx_table_s2_t0_action1 req_rx_rrqlkey_process
#define rx_table_s2_t0_action2 req_rx_cqpt_process

#define rx_table_s2_t2_action req_rx_sqcb1_write_back_process

#define rx_table_s3_t0_action req_rx_rrqptseg_process
#define rx_table_s3_t0_action1 req_rx_eqcb_process

#include "../common-p4+/common_rxdma.p4"

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
    modify_field(phv_global_common_scr._ack, phv_global_common._ack);\
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
        dma_cmd_index                    :    8;
        eq_id                            :   24;
        cq_id                            :   24;
        arm                              :    1;
        wakeup_dpath                     :    1;
        tbl_id                           :    3;
        rsvd                             :    4;
        pad                              :   71;
    }
}

header_type req_rx_rrqwqe_to_cq_info_t {
    fields {
        tbl_id                           :    4;
        dma_cmd_index                    :    8;
        pad                              :  148;
    }
}

header_type req_rx_sqcb1_to_rrqwqe_info_t {
    fields {
        cur_sge_offset                   :   32;
        remaining_payload_bytes          :   32;
        cq_id                            :   24;
        cur_sge_id                       :    8;
        e_rsp_psn                        :   24;
        msn                              :   24;
        rrq_in_progress                  :    1;
        rrq_empty                        :    1;
        timer_active                     :    1;
        dma_cmd_start_index              :    4;
        rrq_cindex                       :    8;
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
        _atomic_aeth                     :    1;
        _aeth                            :    1;
        _completion                      :    1;
        _atomic_ack                      :    1;
        _ack                             :    1;
        _read_resp                       :    1;
        _only                            :    1;
        _last                            :    1;
        _middle                          :    1;
        _first                           :    1;
        _error_disable_qp                :    1;
    }
}

header_type req_rx_to_stage_t {
    fields {
        msn                              :   24;
        bth_psn                          :   24;
        syndrome                         :    8;
        cqcb_base_addr_page_id           :   22;
        log_num_cq_entries               :    4;
        pad                              :   46;
    }
}

header_type req_rx_rrqwqe_to_sge_info_t {
    fields {
        remaining_payload_bytes          :   32;
        cur_sge_offset                   :   32;
        cq_id                            :   24;
        cur_sge_id                       :    8;
        e_rsp_psn                        :   24;
        num_valid_sges                   :    8;
        rrq_in_progress                  :    1;
        is_atomic                        :    1;
        dma_cmd_eop                      :    1;
        dma_cmd_start_index              :    4;
        rrq_cindex                       :    8;
        pad                              :   17;
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
        remaining_payload_bytes          :   32;
        rrq_cindex                       :    8;
        rrq_empty                        :    1;
        need_credits                     :    1;
        dma_cmd_start_index              :    4;
        ecn_set                          :    1;
        p_key                            :   16;
        pad                              :   97;
    }
}

header_type req_rx_rrqlkey_to_ptseg_info_t {
    fields {
        pt_offset                        :   32;
        pt_bytes                         :   16;
        log_page_size                    :    5;
        dma_cmd_start_index              :    8;
        is_atomic                        :    1;
        dma_cmd_eop                      :    1;
        sge_index                        :    8;
        pad                              :   89;
    }
}

header_type req_rx_cqcb_to_eq_info_t {
    fields {
        tbl_id                           :    3;
        rsvd                             :    5;
        dma_cmd_index                    :    8;
        cq_id                            :   24;
        pad                              :  120;
    }
}

header_type req_rx_sqcb1_write_back_info_t {
    fields {
        cur_sge_offset                   :   32;
        e_rsp_psn                        :   24;
        cur_sge_id                       :    8;
        rrq_in_progress                  :    1;
        incr_nxt_to_go_token_id          :    1;
        post_bktrack                     :    1;
        dma_cmd_eop                      :    1;
        last                             :    1;
        num_sges                         :    8;
        tbl_id                           :    8;
        pad                              :   65;
    }
}

header_type req_rx_rrqsge_to_lkey_info_t {
    fields {
        sge_va                           :   64;
        sge_bytes                        :   16;
        log_page_size                    :    5;
        is_atomic                        :    1;
        dma_cmd_eop                      :    1;
        dma_cmd_start_index              :    8;
        sge_index                        :    8;
        cq_dma_cmd_index                 :    8;
        cq_id                            :   24;
        pad                              :   25;
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
metadata req_rx_to_stage_t to_s1_to_stage;
@pragma scratch_metadata
metadata req_rx_to_stage_t to_s1_to_stage_scr;

/**** stage to stage header unions ****/

@pragma pa_header_union ingress common_t0_s2s
metadata req_rx_sqcb1_to_rrqwqe_info_t t0_s2s_sqcb1_to_rrqwqe_info;
@pragma scratch_metadata
metadata req_rx_sqcb1_to_rrqwqe_info_t t0_s2s_sqcb1_to_rrqwqe_info_scr;

@pragma pa_header_union ingress common_t0_s2s
metadata req_rx_rrqwqe_to_sge_info_t t0_s2s_rrqwqe_to_sge_info;
@pragma scratch_metadata
metadata req_rx_rrqwqe_to_sge_info_t t0_s2s_rrqwqe_to_sge_info_scr;

@pragma pa_header_union ingress common_t0_s2s
metadata req_rx_rrqsge_to_lkey_info_t t0_s2s_rrqsge_to_lkey_info;
@pragma scratch_metadata
metadata req_rx_rrqsge_to_lkey_info_t t0_s2s_rrqsge_to_lkey_info_scr;

@pragma pa_header_union ingress common_t0_s2s
metadata req_rx_cqcb_to_pt_info_t t0_s2s_cqcb_to_pt_info;
@pragma scratch_metadata
metadata req_rx_cqcb_to_pt_info_t t0_s2s_cqcb_to_pt_info_scr;

@pragma pa_header_union ingress common_t0_s2s
metadata req_rx_rrqlkey_to_ptseg_info_t t0_s2s_rrqlkey_to_ptseg_info;
@pragma scratch_metadata
metadata req_rx_rrqlkey_to_ptseg_info_t t0_s2s_rrqlkey_to_ptseg_info_scr;

@pragma pa_header_union ingress common_t0_s2s
metadata req_rx_cqcb_to_eq_info_t t0_s2s_cqcb_to_eq_info;
@pragma scratch_metadata
metadata req_rx_cqcb_to_eq_info_t t0_s2s_cqcb_to_eq_info_scr;

@pragma pa_header_union ingress common_t2_s2s
metadata req_rx_ecn_info_t t2_s2s_ecn_info;
@pragma scratch_metadata
metadata req_rx_ecn_info_t t2_s2s_ecn_info_scr;

@pragma pa_header_union ingress common_t2_s2s
metadata req_rx_sqcb1_write_back_info_t t2_s2s_sqcb1_write_back_info;
@pragma scratch_metadata
metadata req_rx_sqcb1_write_back_info_t t2_s2s_sqcb1_write_back_info_scr;

@pragma pa_header_union ingress common_t3_s2s
metadata req_rx_rrqwqe_to_cq_info_t t3_s2s_rrqwqe_to_cq_info;
@pragma scratch_metadata
metadata req_rx_rrqwqe_to_cq_info_t t3_s2s_rrqwqe_to_cq_info_scr;


action req_rx_cqcb_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t3_s2s_rrqwqe_to_cq_info_scr.tbl_id, t3_s2s_rrqwqe_to_cq_info.tbl_id);
    modify_field(t3_s2s_rrqwqe_to_cq_info_scr.dma_cmd_index, t3_s2s_rrqwqe_to_cq_info.dma_cmd_index);
    modify_field(t3_s2s_rrqwqe_to_cq_info_scr.pad, t3_s2s_rrqwqe_to_cq_info.pad);

}
action req_rx_cqpt_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t0_s2s_cqcb_to_pt_info_scr.page_offset, t0_s2s_cqcb_to_pt_info.page_offset);
    modify_field(t0_s2s_cqcb_to_pt_info_scr.page_seg_offset, t0_s2s_cqcb_to_pt_info.page_seg_offset);
    modify_field(t0_s2s_cqcb_to_pt_info_scr.dma_cmd_index, t0_s2s_cqcb_to_pt_info.dma_cmd_index);
    modify_field(t0_s2s_cqcb_to_pt_info_scr.eq_id, t0_s2s_cqcb_to_pt_info.eq_id);
    modify_field(t0_s2s_cqcb_to_pt_info_scr.cq_id, t0_s2s_cqcb_to_pt_info.cq_id);
    modify_field(t0_s2s_cqcb_to_pt_info_scr.arm, t0_s2s_cqcb_to_pt_info.arm);
    modify_field(t0_s2s_cqcb_to_pt_info_scr.wakeup_dpath, t0_s2s_cqcb_to_pt_info.wakeup_dpath);
    modify_field(t0_s2s_cqcb_to_pt_info_scr.tbl_id, t0_s2s_cqcb_to_pt_info.tbl_id);
    modify_field(t0_s2s_cqcb_to_pt_info_scr.rsvd, t0_s2s_cqcb_to_pt_info.rsvd);
    modify_field(t0_s2s_cqcb_to_pt_info_scr.pad, t0_s2s_cqcb_to_pt_info.pad);

}
action req_rx_dcqcn_ecn_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t2_s2s_ecn_info_scr.p_key, t2_s2s_ecn_info.p_key);
    modify_field(t2_s2s_ecn_info_scr.pad, t2_s2s_ecn_info.pad);

}
action req_rx_eqcb_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t0_s2s_cqcb_to_eq_info_scr.tbl_id, t0_s2s_cqcb_to_eq_info.tbl_id);
    modify_field(t0_s2s_cqcb_to_eq_info_scr.rsvd, t0_s2s_cqcb_to_eq_info.rsvd);
    modify_field(t0_s2s_cqcb_to_eq_info_scr.dma_cmd_index, t0_s2s_cqcb_to_eq_info.dma_cmd_index);
    modify_field(t0_s2s_cqcb_to_eq_info_scr.cq_id, t0_s2s_cqcb_to_eq_info.cq_id);
    modify_field(t0_s2s_cqcb_to_eq_info_scr.pad, t0_s2s_cqcb_to_eq_info.pad);

}
action req_rx_rrqlkey_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t0_s2s_rrqsge_to_lkey_info_scr.sge_va, t0_s2s_rrqsge_to_lkey_info.sge_va);
    modify_field(t0_s2s_rrqsge_to_lkey_info_scr.sge_bytes, t0_s2s_rrqsge_to_lkey_info.sge_bytes);
    modify_field(t0_s2s_rrqsge_to_lkey_info_scr.log_page_size, t0_s2s_rrqsge_to_lkey_info.log_page_size);
    modify_field(t0_s2s_rrqsge_to_lkey_info_scr.is_atomic, t0_s2s_rrqsge_to_lkey_info.is_atomic);
    modify_field(t0_s2s_rrqsge_to_lkey_info_scr.dma_cmd_eop, t0_s2s_rrqsge_to_lkey_info.dma_cmd_eop);
    modify_field(t0_s2s_rrqsge_to_lkey_info_scr.dma_cmd_start_index, t0_s2s_rrqsge_to_lkey_info.dma_cmd_start_index);
    modify_field(t0_s2s_rrqsge_to_lkey_info_scr.sge_index, t0_s2s_rrqsge_to_lkey_info.sge_index);
    modify_field(t0_s2s_rrqsge_to_lkey_info_scr.cq_dma_cmd_index, t0_s2s_rrqsge_to_lkey_info.cq_dma_cmd_index);
    modify_field(t0_s2s_rrqsge_to_lkey_info_scr.cq_id, t0_s2s_rrqsge_to_lkey_info.cq_id);
    modify_field(t0_s2s_rrqsge_to_lkey_info_scr.pad, t0_s2s_rrqsge_to_lkey_info.pad);

}
action req_rx_rrqptseg_process () {
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
action req_rx_rrqsge_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t0_s2s_rrqwqe_to_sge_info_scr.remaining_payload_bytes, t0_s2s_rrqwqe_to_sge_info.remaining_payload_bytes);
    modify_field(t0_s2s_rrqwqe_to_sge_info_scr.cur_sge_offset, t0_s2s_rrqwqe_to_sge_info.cur_sge_offset);
    modify_field(t0_s2s_rrqwqe_to_sge_info_scr.cq_id, t0_s2s_rrqwqe_to_sge_info.cq_id);
    modify_field(t0_s2s_rrqwqe_to_sge_info_scr.cur_sge_id, t0_s2s_rrqwqe_to_sge_info.cur_sge_id);
    modify_field(t0_s2s_rrqwqe_to_sge_info_scr.e_rsp_psn, t0_s2s_rrqwqe_to_sge_info.e_rsp_psn);
    modify_field(t0_s2s_rrqwqe_to_sge_info_scr.num_valid_sges, t0_s2s_rrqwqe_to_sge_info.num_valid_sges);
    modify_field(t0_s2s_rrqwqe_to_sge_info_scr.rrq_in_progress, t0_s2s_rrqwqe_to_sge_info.rrq_in_progress);
    modify_field(t0_s2s_rrqwqe_to_sge_info_scr.is_atomic, t0_s2s_rrqwqe_to_sge_info.is_atomic);
    modify_field(t0_s2s_rrqwqe_to_sge_info_scr.dma_cmd_eop, t0_s2s_rrqwqe_to_sge_info.dma_cmd_eop);
    modify_field(t0_s2s_rrqwqe_to_sge_info_scr.dma_cmd_start_index, t0_s2s_rrqwqe_to_sge_info.dma_cmd_start_index);
    modify_field(t0_s2s_rrqwqe_to_sge_info_scr.rrq_cindex, t0_s2s_rrqwqe_to_sge_info.rrq_cindex);
    modify_field(t0_s2s_rrqwqe_to_sge_info_scr.pad, t0_s2s_rrqwqe_to_sge_info.pad);

}
action req_rx_rrqwqe_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_to_stage_scr.msn, to_s1_to_stage.msn);
    modify_field(to_s1_to_stage_scr.bth_psn, to_s1_to_stage.bth_psn);
    modify_field(to_s1_to_stage_scr.syndrome, to_s1_to_stage.syndrome);
    modify_field(to_s1_to_stage_scr.cqcb_base_addr_page_id, to_s1_to_stage.cqcb_base_addr_page_id);
    modify_field(to_s1_to_stage_scr.log_num_cq_entries, to_s1_to_stage.log_num_cq_entries);
    modify_field(to_s1_to_stage_scr.pad, to_s1_to_stage.pad);

    // stage to stage
    modify_field(t0_s2s_sqcb1_to_rrqwqe_info_scr.cur_sge_offset, t0_s2s_sqcb1_to_rrqwqe_info.cur_sge_offset);
    modify_field(t0_s2s_sqcb1_to_rrqwqe_info_scr.remaining_payload_bytes, t0_s2s_sqcb1_to_rrqwqe_info.remaining_payload_bytes);
    modify_field(t0_s2s_sqcb1_to_rrqwqe_info_scr.cq_id, t0_s2s_sqcb1_to_rrqwqe_info.cq_id);
    modify_field(t0_s2s_sqcb1_to_rrqwqe_info_scr.cur_sge_id, t0_s2s_sqcb1_to_rrqwqe_info.cur_sge_id);
    modify_field(t0_s2s_sqcb1_to_rrqwqe_info_scr.e_rsp_psn, t0_s2s_sqcb1_to_rrqwqe_info.e_rsp_psn);
    modify_field(t0_s2s_sqcb1_to_rrqwqe_info_scr.msn, t0_s2s_sqcb1_to_rrqwqe_info.msn);
    modify_field(t0_s2s_sqcb1_to_rrqwqe_info_scr.rrq_in_progress, t0_s2s_sqcb1_to_rrqwqe_info.rrq_in_progress);
    modify_field(t0_s2s_sqcb1_to_rrqwqe_info_scr.rrq_empty, t0_s2s_sqcb1_to_rrqwqe_info.rrq_empty);
    modify_field(t0_s2s_sqcb1_to_rrqwqe_info_scr.timer_active, t0_s2s_sqcb1_to_rrqwqe_info.timer_active);
    modify_field(t0_s2s_sqcb1_to_rrqwqe_info_scr.dma_cmd_start_index, t0_s2s_sqcb1_to_rrqwqe_info.dma_cmd_start_index);
    modify_field(t0_s2s_sqcb1_to_rrqwqe_info_scr.rrq_cindex, t0_s2s_sqcb1_to_rrqwqe_info.rrq_cindex);

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

    // stage to stage
    modify_field(t2_s2s_sqcb1_write_back_info_scr.cur_sge_offset, t2_s2s_sqcb1_write_back_info.cur_sge_offset);
    modify_field(t2_s2s_sqcb1_write_back_info_scr.e_rsp_psn, t2_s2s_sqcb1_write_back_info.e_rsp_psn);
    modify_field(t2_s2s_sqcb1_write_back_info_scr.cur_sge_id, t2_s2s_sqcb1_write_back_info.cur_sge_id);
    modify_field(t2_s2s_sqcb1_write_back_info_scr.rrq_in_progress, t2_s2s_sqcb1_write_back_info.rrq_in_progress);
    modify_field(t2_s2s_sqcb1_write_back_info_scr.incr_nxt_to_go_token_id, t2_s2s_sqcb1_write_back_info.incr_nxt_to_go_token_id);
    modify_field(t2_s2s_sqcb1_write_back_info_scr.post_bktrack, t2_s2s_sqcb1_write_back_info.post_bktrack);
    modify_field(t2_s2s_sqcb1_write_back_info_scr.dma_cmd_eop, t2_s2s_sqcb1_write_back_info.dma_cmd_eop);
    modify_field(t2_s2s_sqcb1_write_back_info_scr.last, t2_s2s_sqcb1_write_back_info.last);
    modify_field(t2_s2s_sqcb1_write_back_info_scr.num_sges, t2_s2s_sqcb1_write_back_info.num_sges);
    modify_field(t2_s2s_sqcb1_write_back_info_scr.tbl_id, t2_s2s_sqcb1_write_back_info.tbl_id);
    modify_field(t2_s2s_sqcb1_write_back_info_scr.pad, t2_s2s_sqcb1_write_back_info.pad);

}

