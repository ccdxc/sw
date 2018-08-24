/***********************************************************************/
/* rdma_aq_rxdma.p4 */
/***********************************************************************/

#include "../common-p4+/common_rxdma_dummy.p4"

//#define common_p4plus_stage0_app_header_table rdma_stage0_table
#define common_p4plus_stage0_app_header_table_action_dummy1 rdma_stage0_aq_feedback_action
#define common_p4plus_stage0_app_header_table_action_dummy2 rdma_stage0_dummy_action
#define common_p4plus_stage0_app_header_table_action_dummy3 rdma_stage0_aq_feedback_action2

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

#define rx_table_s1_t2_action rdma_aq_rx_aqcb_process_dummy
#define rx_table_s1_t3_action rdma_aq_rx_aqwqe_process

#define rx_table_s2_t2_action rdma_aq_rx_aqcb_process_dummy

#define rx_table_s3_t2_action rdma_aq_rx_aqcb_process_dummy

#define rx_table_s4_t2_action rdma_aq_rx_aqcb_process_dummy

#define rx_table_s5_t2_action rdma_aq_rx_aqcb_process_dummy

#define rx_table_s6_t2_action rdma_aq_rx_cqcb_process

//dummy
#define rx_table_s7_t2_action rdma_aq_rx_cqpt_process

#define rx_table_s7_t1_action rdma_aq_rx_eqcb_process

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
        cb_addr                          :   28;
        pt_base_addr_page_id             :   22;
        log_num_pt_entries               :    5;
        _feedback                        :    1;
        _rsvd0                           :   34;
    }
}

header_type aq_rx_aqcb_to_cq_info_t {
    fields {
        aq_id                            :  24;
        cq_id                            :  24;
        status                           :   8;
        error                            :   1;
        pad                              : 103;
    }
}

header_type aq_rx_aqcb_to_wqe_info_t {
    fields {
        rq_id                            :  24;
		rq_tbl_index                     :  32;
		rq_map_count                     :  32;
		rq_dma_addr                      :  64;
        pad                              :   8;
    }
}

header_type aq_rx_cqcb_to_pt_info_t {
    fields {
        page_offset                      :   16;
        page_seg_offset                  :    8;
        cq_id                            :   24;
        fire_eqcb                        :    1;
        no_translate                     :    1;
        no_dma                           :    1;
        cqcb_addr                        :   34;
        pt_next_pg_index                 :   16;
        eqcb_addr                        :   34;
        report_error                     :    1;
        eqe_type                         :    3;
        eqe_code                         :    4;
        pad                              :   17;
    }
}

header_type aq_rx_cqcb_to_eq_info_t {
    fields {
        qid                              :  24;
        eqe_type                         :   3;
        eqe_code                         :   4;
        pad                              :  129;
    }
}

header_type aq_rx_to_stage_t {
    fields {
        cqcb_base_addr_hi                :   24;
        log_num_cq_entries               :    4;
        bth_se                           :    1;
        sqcb_base_addr_hi                :   24;
        rqcb_base_addr_hi                :   24;
        pad                              :   51;
    }
}

@pragma pa_header_union ingress app_header rdma_aq_feedback rdma_bth rdma_aq_feedback_qp 

metadata p4_to_p4plus_roce_bth_header_t rdma_bth;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_header_t rdma_bth_scr;

metadata rdma_aq_completion_feedback_header_t rdma_aq_feedback;
@pragma scratch_metadata
metadata rdma_aq_completion_feedback_header_t rdma_aq_feedback_scr;

metadata rdma_aq_completion_feedback_header_create_qp_t rdma_aq_feedback_qp;
@pragma scratch_metadata
metadata rdma_aq_completion_feedback_header_create_qp_t rdma_aq_feedback_qp_scr;

/**** header unions and scratch ****/

/**** global header unions ****/

@pragma pa_header_union ingress common_global
metadata phv_global_common_t phv_global_common;
@pragma scratch_metadata
metadata phv_global_common_t phv_global_common_scr;

/**** to stage header unions ****/

@pragma pa_header_union ingress to_stage_1
metadata aq_rx_to_stage_t to_s1_info;
@pragma scratch_metadata
metadata aq_rx_to_stage_t to_s1_info_scr;

@pragma pa_header_union ingress to_stage_6
metadata aq_rx_to_stage_t to_s6_info;
@pragma scratch_metadata
metadata aq_rx_to_stage_t to_s6_info_scr;


/**** stage to stage header unions ****/

//Table-0

//Table-1
@pragma pa_header_union ingress common_t1_s2s t1_s2s_cqcb_to_eq_info
metadata aq_rx_cqcb_to_eq_info_t t1_s2s_cqcb_to_eq_info;
@pragma scratch_metadata
metadata aq_rx_cqcb_to_eq_info_t t1_s2s_cqcb_to_eq_info_scr;

//Table-2

@pragma pa_header_union ingress common_t2_s2s t2_s2s_aqcb_to_cq_info t2_s2s_cqcb_to_pt_info 

metadata aq_rx_aqcb_to_cq_info_t t2_s2s_aqcb_to_cq_info;
@pragma scratch_metadata
metadata aq_rx_aqcb_to_cq_info_t t2_s2s_aqcb_to_cq_info_scr;

metadata aq_rx_cqcb_to_pt_info_t t2_s2s_cqcb_to_pt_info;
@pragma scratch_metadata
metadata aq_rx_cqcb_to_pt_info_t t2_s2s_cqcb_to_pt_info_scr;

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
    modify_field(rdma_aq_feedback_scr.cq_num, rdma_aq_feedback.cq_num);
    modify_field(rdma_aq_feedback_scr.status, rdma_aq_feedback.status);
    modify_field(rdma_aq_feedback_scr.error, rdma_aq_feedback.error);
    modify_field(rdma_aq_feedback_scr.op, rdma_aq_feedback.op);
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
    modify_field(rdma_aq_feedback_qp_scr.common_roce_bits, rdma_aq_feedback_qp.common_roce_bits);

    // aq_feedback_header bits
    modify_field(rdma_aq_feedback_qp_scr.aq_comp_common_bits, rdma_aq_feedback_qp.aq_comp_common_bits);

    modify_field(rdma_aq_feedback_qp_scr.rq_cq_id, rdma_aq_feedback_qp.rq_cq_id);
    modify_field(rdma_aq_feedback_qp_scr.rq_depth_log2, rdma_aq_feedback_qp.rq_depth_log2);
    modify_field(rdma_aq_feedback_qp_scr.rq_stride_log2, rdma_aq_feedback_qp.rq_stride_log2);
    modify_field(rdma_aq_feedback_qp_scr.rq_page_size_log2, rdma_aq_feedback_qp.rq_page_size_log2);
    modify_field(rdma_aq_feedback_qp_scr.rq_id, rdma_aq_feedback_qp.rq_id);
    modify_field(rdma_aq_feedback_qp_scr.rq_tbl_index, rdma_aq_feedback_qp.rq_tbl_index);
    modify_field(rdma_aq_feedback_qp_scr.rq_map_count, rdma_aq_feedback_qp.rq_map_count);
    modify_field(rdma_aq_feedback_qp_scr.rq_type_state, rdma_aq_feedback_qp.rq_type_state);
    modify_field(rdma_aq_feedback_qp_scr.pd, rdma_aq_feedback_qp.pd);
    modify_field(rdma_aq_feedback_qp_scr.rq_dma_addr, rdma_aq_feedback_qp.rq_dma_addr);
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


action rdma_aq_rx_aqcb_process_dummy () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t2_s2s_aqcb_to_cq_info_scr.aq_id, t2_s2s_aqcb_to_cq_info.aq_id);
    modify_field(t2_s2s_aqcb_to_cq_info_scr.cq_id, t2_s2s_aqcb_to_cq_info.cq_id);
    modify_field(t2_s2s_aqcb_to_cq_info_scr.status, t2_s2s_aqcb_to_cq_info.status);
    modify_field(t2_s2s_aqcb_to_cq_info_scr.error, t2_s2s_aqcb_to_cq_info.error);
    modify_field(t2_s2s_aqcb_to_cq_info_scr.pad, t2_s2s_aqcb_to_cq_info.pad);

}

action rdma_aq_rx_aqwqe_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_info_scr.sqcb_base_addr_hi, to_s1_info.sqcb_base_addr_hi);
    modify_field(to_s1_info_scr.rqcb_base_addr_hi, to_s1_info.rqcb_base_addr_hi);
    modify_field(to_s1_info_scr.pad, to_s1_info.pad);

    // stage to stage
    modify_field(t3_s2s_aqcb_to_wqe_info_scr.rq_id, t3_s2s_aqcb_to_wqe_info.rq_id);
    modify_field(t3_s2s_aqcb_to_wqe_info_scr.rq_tbl_index, t3_s2s_aqcb_to_wqe_info.rq_tbl_index);
    modify_field(t3_s2s_aqcb_to_wqe_info_scr.rq_map_count, t3_s2s_aqcb_to_wqe_info.rq_map_count);
    modify_field(t3_s2s_aqcb_to_wqe_info_scr.rq_dma_addr, t3_s2s_aqcb_to_wqe_info.rq_dma_addr);
    modify_field(t3_s2s_aqcb_to_wqe_info_scr.pad, t3_s2s_aqcb_to_wqe_info.pad);

}

action rdma_aq_rx_cqcb_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s6_info_scr.cqcb_base_addr_hi, to_s6_info.cqcb_base_addr_hi);
    modify_field(to_s6_info_scr.log_num_cq_entries, to_s6_info.log_num_cq_entries);
    modify_field(to_s6_info_scr.bth_se, to_s6_info.bth_se);
    modify_field(to_s6_info_scr.pad, to_s6_info.pad);

    // stage to stage
    modify_field(t2_s2s_aqcb_to_cq_info_scr.aq_id, t2_s2s_aqcb_to_cq_info.aq_id);
    modify_field(t2_s2s_aqcb_to_cq_info_scr.cq_id, t2_s2s_aqcb_to_cq_info.cq_id);
    modify_field(t2_s2s_aqcb_to_cq_info_scr.status, t2_s2s_aqcb_to_cq_info.status);
    modify_field(t2_s2s_aqcb_to_cq_info_scr.error, t2_s2s_aqcb_to_cq_info.error);
    modify_field(t2_s2s_aqcb_to_cq_info_scr.pad, t2_s2s_aqcb_to_cq_info.pad);

}

action rdma_aq_rx_eqcb_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t1_s2s_cqcb_to_eq_info_scr.qid, t1_s2s_cqcb_to_eq_info.qid);
    modify_field(t1_s2s_cqcb_to_eq_info_scr.pad, t1_s2s_cqcb_to_eq_info.pad);

}

action rdma_aq_rx_cqpt_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // stage to stage
    modify_field(t2_s2s_cqcb_to_pt_info_scr.page_offset, t2_s2s_cqcb_to_pt_info.page_offset);
    modify_field(t2_s2s_cqcb_to_pt_info_scr.page_seg_offset, t2_s2s_cqcb_to_pt_info.page_seg_offset);
    modify_field(t2_s2s_cqcb_to_pt_info_scr.cq_id, t2_s2s_cqcb_to_pt_info.cq_id);
    modify_field(t2_s2s_cqcb_to_pt_info_scr.fire_eqcb, t2_s2s_cqcb_to_pt_info.fire_eqcb);
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
