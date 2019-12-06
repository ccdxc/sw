/***********************************************************************/
/* rdma_aq_rxdma.p4 */
/***********************************************************************/

#include "../common-p4+/common_rxdma_dummy.p4"

//#define common_p4plus_stage0_app_header_table rdma_stage0_table
#define common_p4plus_stage0_app_header_table_action_dummy1 rdma_stage0_aq_feedback_action
#define common_p4plus_stage0_app_header_table_action_dummy2 rdma_stage0_dummy_action
#define common_p4plus_stage0_app_header_table_action_dummy3 rdma_stage0_aq_feedback_action2
#define common_p4plus_stage0_app_header_table_action_dummy4 rdma_stage0_aq_feedback_action3
#define common_p4plus_stage0_app_header_table_action_dummy5 rdma_stage0_aq_feedback_action4

/**** table declarations ****/

#define rx_table_s0_t0 aq_rx_s0_t0
#define rx_table_s0_t1 aq_rx_s0_t1
#define rx_table_s0_t2 aq_rx_s0_t2
#define rx_table_s0_t3 aq_rx_s0_t3

#define rx_table_s1_t0 aq_rx_s1_t0
#define rx_table_s1_t1 aq_rx_s1_t1
#define rx_table_s1_t2 aq_rx_s1_t2
#define rx_table_s1_t3 aq_rx_s1_t3

#define rx_table_s2_t0 aq_rx_s2_t0
#define rx_table_s2_t1 aq_rx_s2_t1
#define rx_table_s2_t2 aq_rx_s2_t2
#define rx_table_s2_t3 aq_rx_s2_t3

#define rx_table_s3_t0 aq_rx_s3_t0
#define rx_table_s3_t1 aq_rx_s3_t1
#define rx_table_s3_t2 aq_rx_s3_t2
#define rx_table_s3_t3 aq_rx_s3_t3

#define rx_table_s4_t0 aq_rx_s4_t0
#define rx_table_s4_t1 aq_rx_s4_t1
#define rx_table_s4_t2 aq_rx_s4_t2
#define rx_table_s4_t3 aq_rx_s4_t3

#define rx_table_s5_t0 aq_rx_s5_t0
#define rx_table_s5_t1 aq_rx_s5_t1
#define rx_table_s5_t2 aq_rx_s5_t2
#define rx_table_s5_t3 aq_rx_s5_t3

#define rx_table_s6_t0 aq_rx_s6_t0
#define rx_table_s6_t1 aq_rx_s6_t1
#define rx_table_s6_t2 aq_rx_s6_t2
#define rx_table_s6_t3 aq_rx_s6_t3

#define rx_table_s7_t0 aq_rx_s7_t0
#define rx_table_s7_t1 aq_rx_s7_t1
#define rx_table_s7_t2 aq_rx_s7_t2
#define rx_table_s7_t3 aq_rx_s7_t3


#define rx_table_s0_t0_action rdma_aq_rx_aqcb_process

#define rx_table_s1_t0_action rdma_aq_rx_cqcb_process_dummy
#define rx_table_s1_t2_action rdma_aq_rx_sqcb_process_dummy
#define rx_table_s1_t3_action rdma_aq_rx_aqwqe_process

#define rx_table_s2_t0_action rdma_aq_rx_cqcb_process_dummy
#define rx_table_s2_t2_action rdma_aq_rx_sqcb_process_dummy

#define rx_table_s3_t0_action rdma_aq_rx_cqcb_process_dummy
#define rx_table_s3_t2_action rdma_aq_rx_sqcb_process_dummy

#define rx_table_s4_t0_action rdma_aq_rx_cqcb_process_dummy
#define rx_table_s4_t2_action rdma_aq_rx_sqcb1_process
#define rx_table_s4_t2_action1 rdma_aq_rx_query_sqcb1_process

#define rx_table_s5_t0_action rdma_aq_rx_cqcb_process_dummy
#define rx_table_s5_t2_action rdma_aq_rx_rqcb1_process

#define rx_table_s6_t2_action rdma_aq_rx_cqcb_process

#define rx_table_s7_t0_action rdma_aq_rx_eqcb_process_t0
#define rx_table_s7_t1_action rdma_aq_rx_eqcb_process_t1

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
    modify_field(phv_global_common_scr._feedback, phv_global_common._feedback);\
    modify_field(phv_global_common_scr._rsvd0, phv_global_common._rsvd0);\

/**** header definitions ****/

header_type phv_global_common_t {
    fields {
        lif                              :   11;
        qid                              :   24;
        qtype                            :    3;
        cb_addr                          :   25;
        pt_base_addr_page_id             :   22;
        log_num_pt_entries               :    5;
        _feedback                        :    1;
        _rsvd0                           :   37;
    }
}

header_type aq_rx_aqcb_to_sqcb1_info_t {
    fields {
        ah_len                   :   8;
        ah_addr                  :  32;
        av_valid                 :   1;
        state                    :   3;
        state_valid              :   1;
        pmtu_log2                :   5;
        pmtu_valid               :   1;            
        rq_id                    :  24;
        sqd_async_notify_en      :   1;
        access_flags_valid       :   1;
        access_flags             :   3;
        cur_state                :   3;
        cur_state_valid          :   1;
        congestion_mgmt_enable   :   1;
        sqcb_addr                :  32;
        pad                      :  43;
    }
}

header_type aq_rx_aqcb_to_wqe_info_t {
    fields {
        rq_id                            :  24;
        rq_tbl_index                     :  32;
        rq_map_count                     :  32;
        rq_dma_addr                      :  64;
        rq_cmb                           :   1;
        rq_spec                          :   1;
        log_wqe_size                     :   5;
        pad                              :   1;
    }
}

header_type aq_rx_cqcb_to_eq_info_t {
    fields {
        async_eq                         :    1;
        pad                              :  159;
    }
}

header_type aq_rx_to_stage_wqe_t {
    fields {
        rqcb_base_addr_hi                :   24;
        prefetch_base_addr_page_id       :   22;
        log_prefetch_buf_size            :    5;
        pad                              :   77;
    }
}

header_type aq_rx_to_stage_sqcb1_t {
    fields {
        sqcb_base_addr_hi        :  24;
        rqcb_base_addr_hi        :  24;
        rrq_base_addr            :  32;
        rrq_depth_log2           :   5;
        rrq_valid                :   1;
        err_retry_count          :   3;
        err_retry_count_valid    :   1;
        tx_psn_valid             :   1;
        tx_psn                   :  24;
        rnr_retry_count          :   3;
        rnr_retry_count_valid    :   1;
        pad                      :   9;
    }
}

header_type aq_rx_to_stage_rqcb1_t {
    fields {
        cqcb_base_addr_hi        :    24;
        cq_id                    :    24;
        q_key_rsq_base_addr      :    32;
        q_key_valid              :     1;
        rsq_depth_log2           :     5;
        rsq_valid                :     1;
        rq_psn                   :    24;
        rq_psn_valid             :     1;
        pad                      :    16;
    }
}

header_type aq_rx_to_stage_cqcb_t {
    fields {
        cqcb_base_addr_hi                :   24;
        log_num_cq_entries               :    4;
        wqe_id                           :   16;
        aqcb_addr                        :   28;
        cq_id                            :   24;
        status                           :    8;
        op                               :    8;
        error                            :    1;
        pad                              :   15;
    }
}

@pragma pa_header_union ingress app_header rdma_aq_feedback rdma_bth rdma_aq_feedback_cqp rdma_aq_feedback_mqp rdma_aq_feedback_qdqp

metadata p4_to_p4plus_roce_bth_header_t rdma_bth;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_header_t rdma_bth_scr;

metadata rdma_aq_completion_feedback_header_t rdma_aq_feedback;
@pragma scratch_metadata
metadata rdma_aq_completion_feedback_header_t rdma_aq_feedback_scr;

metadata rdma_aq_completion_feedback_header_create_qp_t rdma_aq_feedback_cqp;
@pragma scratch_metadata
metadata rdma_aq_completion_feedback_header_create_qp_t rdma_aq_feedback_cqp_scr;

metadata rdma_aq_completion_feedback_header_modify_qp_t rdma_aq_feedback_mqp;
@pragma scratch_metadata
metadata rdma_aq_completion_feedback_header_modify_qp_t rdma_aq_feedback_mqp_scr;

metadata rdma_aq_completion_feedback_header_query_destroy_qp_t rdma_aq_feedback_qdqp;
@pragma scratch_metadata
metadata rdma_aq_completion_feedback_header_query_destroy_qp_t rdma_aq_feedback_qdqp_scr;

/**** header unions and scratch ****/

/**** global header unions ****/

@pragma pa_header_union ingress common_global
metadata phv_global_common_t phv_global_common;
@pragma scratch_metadata
metadata phv_global_common_t phv_global_common_scr;

/**** to stage header unions ****/

@pragma pa_header_union ingress to_stage_1
metadata aq_rx_to_stage_wqe_t to_s1_info;
@pragma scratch_metadata
metadata aq_rx_to_stage_wqe_t to_s1_info_scr;

@pragma pa_header_union ingress to_stage_4
metadata aq_rx_to_stage_sqcb1_t to_s4_info;
@pragma scratch_metadata
metadata aq_rx_to_stage_sqcb1_t to_s4_info_scr;

@pragma pa_header_union ingress to_stage_5
metadata aq_rx_to_stage_rqcb1_t to_s5_info;
@pragma scratch_metadata
metadata aq_rx_to_stage_rqcb1_t to_s5_info_scr;

@pragma pa_header_union ingress to_stage_6
metadata aq_rx_to_stage_cqcb_t to_s6_info;
@pragma scratch_metadata
metadata aq_rx_to_stage_cqcb_t to_s6_info_scr;


/**** stage to stage header unions ****/

//Table-0
@pragma pa_header_union ingress common_t0_s2s t0_s2s_cqcb_to_eq_info
metadata aq_rx_cqcb_to_eq_info_t t0_s2s_cqcb_to_eq_info;
@pragma scratch_metadata
metadata aq_rx_cqcb_to_eq_info_t t0_s2s_cqcb_to_eq_info_scr;

//Table-1
@pragma pa_header_union ingress common_t1_s2s t1_s2s_cqcb_to_eq_info
metadata aq_rx_cqcb_to_eq_info_t t1_s2s_cqcb_to_eq_info;
@pragma scratch_metadata
metadata aq_rx_cqcb_to_eq_info_t t1_s2s_cqcb_to_eq_info_scr;

//Table-2

@pragma pa_header_union ingress common_t2_s2s t2_s2s_aqcb_to_sqcb1_info t2_s2s_sqcb1_to_rqcb1_info

metadata aq_rx_aqcb_to_sqcb1_info_t t2_s2s_aqcb_to_sqcb1_info;
@pragma scratch_metadata
metadata aq_rx_aqcb_to_sqcb1_info_t t2_s2s_aqcb_to_sqcb1_info_scr;

metadata aq_rx_aqcb_to_sqcb1_info_t t2_s2s_sqcb1_to_rqcb1_info;
@pragma scratch_metadata
metadata aq_rx_aqcb_to_sqcb1_info_t t2_s2s_sqcb1_to_rqcb1_info_scr;

//Table-3
@pragma pa_header_union ingress common_t3_s2s t3_s2s_aqcb_to_wqe_info

metadata aq_rx_aqcb_to_wqe_info_t t3_s2s_aqcb_to_wqe_info;
@pragma scratch_metadata
metadata aq_rx_aqcb_to_wqe_info_t t3_s2s_aqcb_to_wqe_info_scr;


/*
 * Stage 0 table 0 aq_feedback action
 */
action rdma_stage0_aq_feedback_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // We only need raw-flags in this piece. Can be pruned further
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


    // from app header
    modify_field(rdma_aq_feedback_scr.common_header_bits, rdma_aq_feedback.common_header_bits);

    // aq_feedback_header bits
    modify_field(rdma_aq_feedback_scr.feedback_type, rdma_aq_feedback.feedback_type);
    modify_field(rdma_aq_feedback_scr.wqe_id, rdma_aq_feedback.wqe_id);
    modify_field(rdma_aq_feedback_scr.status, rdma_aq_feedback.status);
    modify_field(rdma_aq_feedback_scr.op, rdma_aq_feedback.op);
    modify_field(rdma_aq_feedback_scr.error, rdma_aq_feedback.error);
}

action rdma_stage0_aq_feedback_action2 () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // We only need raw-flags in this piece. Can be pruned further
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


    // from app header
    modify_field(rdma_aq_feedback_cqp_scr.common_roce_bits, rdma_aq_feedback_cqp.common_roce_bits);

    // aq_feedback_header bits
    modify_field(rdma_aq_feedback_cqp_scr.aq_comp_common_bits, rdma_aq_feedback_cqp.aq_comp_common_bits);

    modify_field(rdma_aq_feedback_cqp_scr.rq_cq_id, rdma_aq_feedback_cqp.rq_cq_id);
    modify_field(rdma_aq_feedback_cqp_scr.rq_depth_log2, rdma_aq_feedback_cqp.rq_depth_log2);
    modify_field(rdma_aq_feedback_cqp_scr.rq_stride_log2, rdma_aq_feedback_cqp.rq_stride_log2);
    modify_field(rdma_aq_feedback_cqp_scr.rq_page_size_log2, rdma_aq_feedback_cqp.rq_page_size_log2);
    modify_field(rdma_aq_feedback_cqp_scr.rq_id, rdma_aq_feedback_cqp.rq_id);
    modify_field(rdma_aq_feedback_cqp_scr.rq_tbl_index, rdma_aq_feedback_cqp.rq_tbl_index);
    modify_field(rdma_aq_feedback_cqp_scr.rq_map_count, rdma_aq_feedback_cqp.rq_map_count);
    modify_field(rdma_aq_feedback_cqp_scr.rq_type_state, rdma_aq_feedback_cqp.rq_type_state);
    modify_field(rdma_aq_feedback_cqp_scr.pd, rdma_aq_feedback_cqp.pd);
    modify_field(rdma_aq_feedback_cqp_scr.rq_dma_addr, rdma_aq_feedback_cqp.rq_dma_addr);
    modify_field(rdma_aq_feedback_cqp_scr.rq_cmb, rdma_aq_feedback_cqp.rq_cmb);
    modify_field(rdma_aq_feedback_cqp_scr.rq_spec, rdma_aq_feedback_cqp.rq_spec);
    modify_field(rdma_aq_feedback_cqp_scr.qp_privileged, rdma_aq_feedback_cqp.qp_privileged);
    modify_field(rdma_aq_feedback_cqp_scr.log_pmtu, rdma_aq_feedback_cqp.log_pmtu);
    modify_field(rdma_aq_feedback_cqp_scr.access_flags, rdma_aq_feedback_cqp.access_flags);
    modify_field(rdma_aq_feedback_cqp_scr.pid, rdma_aq_feedback_cqp.pid);
}


action rdma_stage0_aq_feedback_action3 () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // We only need raw-flags in this piece. Can be pruned further
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


    // from app header
    modify_field(rdma_aq_feedback_mqp_scr.common_roce_bits, rdma_aq_feedback_mqp.common_roce_bits);

    // aq_feedback_header bits
    modify_field(rdma_aq_feedback_mqp_scr.aq_comp_common_bits, rdma_aq_feedback_mqp.aq_comp_common_bits);

    modify_field(rdma_aq_feedback_mqp_scr.rq_psn, rdma_aq_feedback_mqp.rq_psn);
    modify_field(rdma_aq_feedback_mqp_scr.rq_psn_valid, rdma_aq_feedback_mqp.rq_psn_valid);

    modify_field(rdma_aq_feedback_mqp_scr.rq_id, rdma_aq_feedback_mqp.rq_id);
    modify_field(rdma_aq_feedback_mqp_scr.ah_len, rdma_aq_feedback_mqp.ah_len);
    modify_field(rdma_aq_feedback_mqp_scr.ah_addr, rdma_aq_feedback_mqp.ah_addr);
    modify_field(rdma_aq_feedback_mqp_scr.rrq_base_addr, rdma_aq_feedback_mqp.rrq_base_addr);
    modify_field(rdma_aq_feedback_mqp_scr.rrq_depth_log2, rdma_aq_feedback_mqp.rrq_depth_log2);
    modify_field(rdma_aq_feedback_mqp_scr.q_key_rsq_base_addr, rdma_aq_feedback_mqp.q_key_rsq_base_addr);
    modify_field(rdma_aq_feedback_mqp_scr.q_key_valid, rdma_aq_feedback_mqp.q_key_valid);
    modify_field(rdma_aq_feedback_mqp_scr.rsq_depth_log2, rdma_aq_feedback_mqp.rsq_depth_log2);
    modify_field(rdma_aq_feedback_mqp_scr.av_valid, rdma_aq_feedback_mqp.av_valid);
    modify_field(rdma_aq_feedback_mqp_scr.rsq_valid, rdma_aq_feedback_mqp.rsq_valid);
    modify_field(rdma_aq_feedback_mqp_scr.rrq_valid, rdma_aq_feedback_mqp.rrq_valid);
    modify_field(rdma_aq_feedback_mqp_scr.state, rdma_aq_feedback_mqp.state);
    modify_field(rdma_aq_feedback_mqp_scr.state_valid, rdma_aq_feedback_mqp.state_valid);
    modify_field(rdma_aq_feedback_mqp_scr.err_retry_count, rdma_aq_feedback_mqp.err_retry_count);

    modify_field(rdma_aq_feedback_mqp_scr.err_retry_count_valid, rdma_aq_feedback_mqp.err_retry_count_valid);
    modify_field(rdma_aq_feedback_mqp_scr.tx_psn_valid, rdma_aq_feedback_mqp.tx_psn_valid);
    modify_field(rdma_aq_feedback_mqp_scr.tx_psn, rdma_aq_feedback_mqp.tx_psn);
    modify_field(rdma_aq_feedback_mqp_scr.rnr_retry_count, rdma_aq_feedback_mqp.rnr_retry_count);
    modify_field(rdma_aq_feedback_mqp_scr.rnr_retry_valid, rdma_aq_feedback_mqp.rnr_retry_valid);
    modify_field(rdma_aq_feedback_mqp_scr.pmtu_log2, rdma_aq_feedback_mqp.pmtu_log2);
    modify_field(rdma_aq_feedback_mqp_scr.pmtu_valid, rdma_aq_feedback_mqp.pmtu_valid);
    modify_field(rdma_aq_feedback_mqp_scr.sqd_async_notify_en, rdma_aq_feedback_mqp.sqd_async_notify_en);
    modify_field(rdma_aq_feedback_mqp_scr.access_flags_valid, rdma_aq_feedback_mqp.access_flags_valid);
    modify_field(rdma_aq_feedback_mqp_scr.access_flags, rdma_aq_feedback_mqp.access_flags);
    modify_field(rdma_aq_feedback_mqp_scr.cur_state, rdma_aq_feedback_mqp.cur_state);
    modify_field(rdma_aq_feedback_mqp_scr.cur_state_valid, rdma_aq_feedback_mqp.cur_state_valid);
    modify_field(rdma_aq_feedback_mqp_scr.congestion_mgmt_enable, rdma_aq_feedback_mqp.congestion_mgmt_enable);
}

action rdma_stage0_aq_feedback_action4 () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // We only need raw-flags in this piece. Can be pruned further
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


    // from app header
    modify_field(rdma_aq_feedback_qdqp_scr.common_roce_bits, rdma_aq_feedback_qdqp.common_roce_bits);

    // aq_feedback_header bits
    modify_field(rdma_aq_feedback_qdqp_scr.aq_comp_common_bits, rdma_aq_feedback_qdqp.aq_comp_common_bits);

    modify_field(rdma_aq_feedback_qdqp_scr.rq_id, rdma_aq_feedback_qdqp.rq_id);
    modify_field(rdma_aq_feedback_qdqp_scr.dma_addr, rdma_aq_feedback_qdqp.dma_addr);
}

/*
 * Stage 0 table 0 dummy action
 */
action rdma_stage0_dummy_action () {
    // k + i for stage 0

    // We only need raw-flags in this piece. Can be pruned further
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

}

action rdma_aq_rx_aqcb_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
}


action rdma_aq_rx_cqcb_process_dummy () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
}

action rdma_aq_rx_sqcb_process_dummy () {
    GENERATE_GLOBAL_K

}

action rdma_aq_rx_aqwqe_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_info_scr.rqcb_base_addr_hi, to_s1_info.rqcb_base_addr_hi);
    modify_field(to_s1_info_scr.prefetch_base_addr_page_id, to_s1_info.prefetch_base_addr_page_id);
    modify_field(to_s1_info_scr.log_prefetch_buf_size, to_s1_info.log_prefetch_buf_size);
    modify_field(to_s1_info_scr.pad, to_s1_info.pad);

    // stage to stage
    modify_field(t3_s2s_aqcb_to_wqe_info_scr.rq_id, t3_s2s_aqcb_to_wqe_info.rq_id);
    modify_field(t3_s2s_aqcb_to_wqe_info_scr.rq_tbl_index, t3_s2s_aqcb_to_wqe_info.rq_tbl_index);
    modify_field(t3_s2s_aqcb_to_wqe_info_scr.rq_map_count, t3_s2s_aqcb_to_wqe_info.rq_map_count);
    modify_field(t3_s2s_aqcb_to_wqe_info_scr.rq_dma_addr, t3_s2s_aqcb_to_wqe_info.rq_dma_addr);
    modify_field(t3_s2s_aqcb_to_wqe_info_scr.rq_cmb, t3_s2s_aqcb_to_wqe_info.rq_cmb);
    modify_field(t3_s2s_aqcb_to_wqe_info_scr.rq_spec, t3_s2s_aqcb_to_wqe_info.rq_spec);
    modify_field(t3_s2s_aqcb_to_wqe_info_scr.log_wqe_size, t3_s2s_aqcb_to_wqe_info.log_wqe_size);
    modify_field(t3_s2s_aqcb_to_wqe_info_scr.pad, t3_s2s_aqcb_to_wqe_info.pad);

}

action rdma_aq_rx_query_sqcb1_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_info_scr.rqcb_base_addr_hi, to_s4_info.rqcb_base_addr_hi);
    modify_field(to_s4_info_scr.sqcb_base_addr_hi, to_s4_info.sqcb_base_addr_hi);

    //stage to stage
    modify_field(t2_s2s_aqcb_to_sqcb1_info_scr.rq_id, t2_s2s_aqcb_to_sqcb1_info.rq_id);
}

action rdma_aq_rx_sqcb1_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_info_scr.rqcb_base_addr_hi, to_s4_info.rqcb_base_addr_hi);
    modify_field(to_s4_info_scr.sqcb_base_addr_hi, to_s4_info.sqcb_base_addr_hi);
    modify_field(to_s4_info_scr.rrq_base_addr, to_s4_info.rrq_base_addr);
    modify_field(to_s4_info_scr.rrq_depth_log2, to_s4_info.rrq_depth_log2);
    modify_field(to_s4_info_scr.rrq_valid, to_s4_info.rrq_valid);
    modify_field(to_s4_info_scr.err_retry_count, to_s4_info.err_retry_count);
    modify_field(to_s4_info_scr.err_retry_count_valid, to_s4_info.err_retry_count_valid);
    modify_field(to_s4_info_scr.rnr_retry_count, to_s4_info.rnr_retry_count);
    modify_field(to_s4_info_scr.rnr_retry_count_valid, to_s4_info.rnr_retry_count_valid);
    modify_field(to_s4_info_scr.tx_psn_valid, to_s4_info.tx_psn_valid);
    modify_field(to_s4_info_scr.tx_psn, to_s4_info.tx_psn);
    modify_field(to_s4_info_scr.pad, to_s4_info.pad);

    //stage to stage
    modify_field(t2_s2s_aqcb_to_sqcb1_info_scr.ah_len, t2_s2s_aqcb_to_sqcb1_info.ah_len);    
    modify_field(t2_s2s_aqcb_to_sqcb1_info_scr.ah_addr, t2_s2s_aqcb_to_sqcb1_info.ah_addr);    
    modify_field(t2_s2s_aqcb_to_sqcb1_info_scr.av_valid, t2_s2s_aqcb_to_sqcb1_info.av_valid);    
    modify_field(t2_s2s_aqcb_to_sqcb1_info_scr.state, t2_s2s_aqcb_to_sqcb1_info.state);    
    modify_field(t2_s2s_aqcb_to_sqcb1_info_scr.state_valid, t2_s2s_aqcb_to_sqcb1_info.state_valid);    
    modify_field(t2_s2s_aqcb_to_sqcb1_info_scr.pmtu_log2, t2_s2s_aqcb_to_sqcb1_info.pmtu_log2);    
    modify_field(t2_s2s_aqcb_to_sqcb1_info_scr.pmtu_valid, t2_s2s_aqcb_to_sqcb1_info.pmtu_valid);    
    modify_field(t2_s2s_aqcb_to_sqcb1_info_scr.rq_id, t2_s2s_aqcb_to_sqcb1_info.rq_id);
    modify_field(t2_s2s_aqcb_to_sqcb1_info_scr.sqd_async_notify_en, t2_s2s_aqcb_to_sqcb1_info.sqd_async_notify_en);
    modify_field(t2_s2s_aqcb_to_sqcb1_info_scr.access_flags_valid, t2_s2s_aqcb_to_sqcb1_info.access_flags_valid);
    modify_field(t2_s2s_aqcb_to_sqcb1_info_scr.access_flags, t2_s2s_aqcb_to_sqcb1_info.access_flags);
    modify_field(t2_s2s_aqcb_to_sqcb1_info_scr.cur_state, t2_s2s_aqcb_to_sqcb1_info.cur_state);    
    modify_field(t2_s2s_aqcb_to_sqcb1_info_scr.cur_state_valid, t2_s2s_aqcb_to_sqcb1_info.cur_state_valid);    
    modify_field(t2_s2s_aqcb_to_sqcb1_info_scr.congestion_mgmt_enable, t2_s2s_aqcb_to_sqcb1_info.congestion_mgmt_enable);    
    modify_field(t2_s2s_aqcb_to_sqcb1_info_scr.pad, t2_s2s_aqcb_to_sqcb1_info.pad);    
}

action rdma_aq_rx_rqcb1_process () {
    // from ki global
    GENERATE_GLOBAL_K

    //to stage
    modify_field(to_s5_info_scr.q_key_rsq_base_addr, to_s5_info.q_key_rsq_base_addr);
    modify_field(to_s5_info_scr.q_key_valid, to_s5_info.q_key_valid);
    modify_field(to_s5_info_scr.rsq_depth_log2, to_s5_info.rsq_depth_log2);
    modify_field(to_s5_info_scr.rsq_valid, to_s5_info.rsq_valid);
    modify_field(to_s5_info_scr.rq_psn, to_s5_info.rq_psn);
    modify_field(to_s5_info_scr.rq_psn_valid, to_s5_info.rq_psn_valid);
    modify_field(to_s5_info_scr.pad, to_s5_info.pad);    

    //stage to stage
    modify_field(t2_s2s_sqcb1_to_rqcb1_info_scr.ah_len, t2_s2s_sqcb1_to_rqcb1_info.ah_len);    
    modify_field(t2_s2s_sqcb1_to_rqcb1_info_scr.ah_addr, t2_s2s_sqcb1_to_rqcb1_info.ah_addr);    
    modify_field(t2_s2s_sqcb1_to_rqcb1_info_scr.av_valid, t2_s2s_sqcb1_to_rqcb1_info.av_valid);    
    modify_field(t2_s2s_sqcb1_to_rqcb1_info_scr.state, t2_s2s_sqcb1_to_rqcb1_info.state);    
    modify_field(t2_s2s_sqcb1_to_rqcb1_info_scr.state_valid, t2_s2s_sqcb1_to_rqcb1_info.state_valid);    
    modify_field(t2_s2s_sqcb1_to_rqcb1_info_scr.pmtu_log2, t2_s2s_sqcb1_to_rqcb1_info.pmtu_log2);    
    modify_field(t2_s2s_sqcb1_to_rqcb1_info_scr.pmtu_valid, t2_s2s_sqcb1_to_rqcb1_info.pmtu_valid);    
    modify_field(t2_s2s_sqcb1_to_rqcb1_info_scr.rq_id, t2_s2s_sqcb1_to_rqcb1_info.rq_id);
    modify_field(t2_s2s_sqcb1_to_rqcb1_info_scr.sqd_async_notify_en, t2_s2s_sqcb1_to_rqcb1_info.sqd_async_notify_en);
    modify_field(t2_s2s_sqcb1_to_rqcb1_info_scr.access_flags_valid, t2_s2s_sqcb1_to_rqcb1_info.access_flags_valid);
    modify_field(t2_s2s_sqcb1_to_rqcb1_info_scr.access_flags, t2_s2s_sqcb1_to_rqcb1_info.access_flags);
    modify_field(t2_s2s_sqcb1_to_rqcb1_info_scr.cur_state, t2_s2s_sqcb1_to_rqcb1_info.cur_state);
    modify_field(t2_s2s_sqcb1_to_rqcb1_info_scr.cur_state_valid, t2_s2s_sqcb1_to_rqcb1_info.cur_state_valid);
    modify_field(t2_s2s_sqcb1_to_rqcb1_info_scr.sqcb_addr, t2_s2s_sqcb1_to_rqcb1_info.sqcb_addr);
    modify_field(t2_s2s_sqcb1_to_rqcb1_info_scr.congestion_mgmt_enable, t2_s2s_sqcb1_to_rqcb1_info.congestion_mgmt_enable);    
    modify_field(t2_s2s_sqcb1_to_rqcb1_info_scr.pad, t2_s2s_sqcb1_to_rqcb1_info.pad);
}

action rdma_aq_rx_cqcb_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s6_info_scr.cqcb_base_addr_hi, to_s6_info.cqcb_base_addr_hi);
    modify_field(to_s6_info_scr.log_num_cq_entries, to_s6_info.log_num_cq_entries);
    modify_field(to_s6_info_scr.wqe_id, to_s6_info.wqe_id);
    modify_field(to_s6_info_scr.aqcb_addr, to_s6_info.aqcb_addr);    

    modify_field(to_s6_info_scr.cq_id, to_s6_info.cq_id);
    modify_field(to_s6_info_scr.status, to_s6_info.status);
    modify_field(to_s6_info_scr.error, to_s6_info.error);
    modify_field(to_s6_info_scr.op, to_s6_info.op);
    modify_field(to_s6_info_scr.pad, to_s6_info.pad);

}

action rdma_aq_rx_eqcb_process_t0 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t0_s2s_cqcb_to_eq_info_scr.async_eq, t0_s2s_cqcb_to_eq_info.async_eq);
    modify_field(t0_s2s_cqcb_to_eq_info_scr.pad, t0_s2s_cqcb_to_eq_info.pad);

}

action rdma_aq_rx_eqcb_process_t1 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t1_s2s_cqcb_to_eq_info_scr.async_eq, t1_s2s_cqcb_to_eq_info.async_eq);
    modify_field(t1_s2s_cqcb_to_eq_info_scr.pad, t1_s2s_cqcb_to_eq_info.pad);

}
