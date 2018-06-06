/***********************************************************************/
/* rdma_cq_rxdma.p4 */
/***********************************************************************/

#include "../common-p4+/common_rxdma_dummy.p4"

//#define common_p4plus_stage0_app_header_table rdma_stage0_table
#define common_p4plus_stage0_app_header_table_action_dummy1 rdma_stage0_cq_feedback_action
#define common_p4plus_stage0_app_header_table_action_dummy2 rdma_stage0_dummy_action

/**** table declarations ****/

#define rx_table_s0_t0 cq_rx_s0_t0
#define rx_table_s0_t1 cq_rx_s0_t1
#define rx_table_s0_t2 cq_rx_s0_t2
#define rx_table_s0_t3 cq_rx_s0_t3

#define rx_table_s1_t0 cq_rx_s1_t0
#define rx_table_s1_t1 cq_rx_s1_t1
#define rx_table_s1_t2 cq_rx_s1_t2
#define rx_table_s1_t3 cq_rx_s1_t3

#define rx_table_s2_t0 cq_rx_s2_t0
#define rx_table_s2_t1 cq_rx_s2_t1
#define rx_table_s2_t2 cq_rx_s2_t2
#define rx_table_s2_t3 cq_rx_s2_t3

#define rx_table_s3_t0 cq_rx_s3_t0
#define rx_table_s3_t1 cq_rx_s3_t1
#define rx_table_s3_t2 cq_rx_s3_t2
#define rx_table_s3_t3 cq_rx_s3_t3

#define rx_table_s4_t0 cq_rx_s4_t0
#define rx_table_s4_t1 cq_rx_s4_t1
#define rx_table_s4_t2 cq_rx_s4_t2
#define rx_table_s4_t3 cq_rx_s4_t3

#define rx_table_s5_t0 cq_rx_s5_t0
#define rx_table_s5_t1 cq_rx_s5_t1
#define rx_table_s5_t2 cq_rx_s5_t2
#define rx_table_s5_t3 cq_rx_s5_t3

#define rx_table_s6_t0 cq_rx_s6_t0
#define rx_table_s6_t1 cq_rx_s6_t1
#define rx_table_s6_t2 cq_rx_s6_t2
#define rx_table_s6_t3 cq_rx_s6_t3

#define rx_table_s7_t0 cq_rx_s7_t0
#define rx_table_s7_t1 cq_rx_s7_t1
#define rx_table_s7_t2 cq_rx_s7_t2
#define rx_table_s7_t3 cq_rx_s7_t3


#define rx_table_s0_t0_action cq_rx_cqcb0_process

#define rx_table_s1_t2_action cq_rx_cqcb_process_dummy

#define rx_table_s2_t2_action cq_rx_cqcb_process_dummy

#define rx_table_s3_t2_action cq_rx_cqcb_process_dummy

#define rx_table_s4_t2_action cq_rx_cqcb_process_dummy

#define rx_table_s5_t2_action cq_rx_cqcb_process

//dummy
#define rx_table_s6_t2_action cq_rx_eqcb_process

#define rx_table_s7_t2_action cq_rx_eqcb_process

#include "../common-p4+/common_rxdma.p4"
#include "./rdma_rxdma_headers.p4"

/**** Macros ****/

#define GENERATE_GLOBAL_K \
    modify_field(phv_global_common_scr.lif, phv_global_common.lif);\
    modify_field(phv_global_common_scr.qid, phv_global_common.qid);\
    modify_field(phv_global_common_scr.qtype, phv_global_common.qtype);\
    modify_field(phv_global_common_scr.cb_addr, phv_global_common.cb_addr);\
    modify_field(phv_global_common_scr.pad, phv_global_common.pad);\
    modify_field(phv_global_common_scr._feedback, phv_global_common._feedback);\
    modify_field(phv_global_common_scr._rsvd0, phv_global_common._rsvd0);\

/**** header definitions ****/

header_type phv_global_common_t {
    fields {
        lif                              :   11;
        qid                              :   24;
        qtype                            :    3;
        cb_addr                          :   25;
        pad                              :   49;
        _feedback                        :    1;
        _rsvd0                           :   15;
    }
}

header_type cq_rx_cqcb0_to_cq_info_t {
    fields {
        cq_id                            :  24;
        cindex                           :  16;
        arm                              :   1;
        sarm                             :   1;
        pad                              :  96;
    }
}

header_type cq_rx_cqcb_to_eq_info_t {
    fields {
        cq_id                            :  24;
        eqcb_addr                        :  64;
        pad                              :  32;
    }
}

header_type cq_rx_to_stage_eq_info_t {
    fields {
        cqcb_base_addr_hi                :   24;
        log_num_cq_entries               :    4;
        pad                              :  100;
    }
}

@pragma pa_header_union ingress app_header rdma_cq_feedback rdma_bth

metadata p4_to_p4plus_roce_bth_header_t rdma_bth;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_header_t rdma_bth_scr;

metadata rdma_cq_feedback_header_t rdma_cq_feedback;
@pragma scratch_metadata
metadata rdma_cq_feedback_header_t rdma_cq_feedback_scr;


/**** header unions and scratch ****/

/**** global header unions ****/

@pragma pa_header_union ingress common_global
metadata phv_global_common_t phv_global_common;
@pragma scratch_metadata
metadata phv_global_common_t phv_global_common_scr;

/**** to stage header unions ****/

@pragma pa_header_union ingress to_stage_5
metadata cq_rx_to_stage_eq_info_t to_s5_info;
@pragma scratch_metadata
metadata cq_rx_to_stage_eq_info_t to_s5_info_scr;


/**** stage to stage header unions ****/

//Table-0

//Table-1

//Table-2

@pragma pa_header_union ingress common_t2_s2s t2_s2s_cqcb0_to_cq_info t2_s2s_cqcb_to_eq_info

metadata cq_rx_cqcb0_to_cq_info_t t2_s2s_cqcb0_to_cq_info;
@pragma scratch_metadata
metadata cq_rx_cqcb0_to_cq_info_t t2_s2s_cqcb0_to_cq_info_scr;

metadata cq_rx_cqcb_to_eq_info_t t2_s2s_cqcb_to_eq_info;
@pragma scratch_metadata
metadata cq_rx_cqcb_to_eq_info_t t2_s2s_cqcb_to_eq_info_scr;

//Table-3

/*
 * Stage 0 table 0 cq_feedback action
 */
action rdma_stage0_cq_feedback_action () {
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
    modify_field(rdma_cq_feedback_scr.common_header_bits, rdma_cq_feedback.common_header_bits);

    // cq_feedback_header bits
    modify_field(rdma_cq_feedback_scr.feedback_type, rdma_cq_feedback.feedback_type);
    modify_field(rdma_cq_feedback_scr.cindex, rdma_cq_feedback.cindex);
    modify_field(rdma_cq_feedback_scr.arm, rdma_cq_feedback.arm);
    modify_field(rdma_cq_feedback_scr.sarm, rdma_cq_feedback.sarm);
    modify_field(rdma_cq_feedback_scr.pad, rdma_cq_feedback.pad);
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

action cq_rx_cqcb0_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
}


action cq_rx_cqcb_process_dummy () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t2_s2s_cqcb0_to_cq_info_scr.cq_id, t2_s2s_cqcb0_to_cq_info.cq_id);
    modify_field(t2_s2s_cqcb0_to_cq_info_scr.cindex, t2_s2s_cqcb0_to_cq_info.cindex);
    modify_field(t2_s2s_cqcb0_to_cq_info_scr.arm, t2_s2s_cqcb0_to_cq_info.arm);
    modify_field(t2_s2s_cqcb0_to_cq_info_scr.sarm, t2_s2s_cqcb0_to_cq_info.sarm);
    modify_field(t2_s2s_cqcb0_to_cq_info_scr.pad, t2_s2s_cqcb0_to_cq_info.pad);

}

action cq_rx_cqcb_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s5_info_scr.cqcb_base_addr_hi, to_s5_info.cqcb_base_addr_hi);
    modify_field(to_s5_info_scr.log_num_cq_entries, to_s5_info.log_num_cq_entries);
    modify_field(to_s5_info_scr.pad, to_s5_info.pad);

    // stage to stage
    modify_field(t2_s2s_cqcb0_to_cq_info_scr.cq_id, t2_s2s_cqcb0_to_cq_info.cq_id);
    modify_field(t2_s2s_cqcb0_to_cq_info_scr.cindex, t2_s2s_cqcb0_to_cq_info.cindex);
    modify_field(t2_s2s_cqcb0_to_cq_info_scr.arm, t2_s2s_cqcb0_to_cq_info.arm);
    modify_field(t2_s2s_cqcb0_to_cq_info_scr.sarm, t2_s2s_cqcb0_to_cq_info.sarm);
    modify_field(t2_s2s_cqcb0_to_cq_info_scr.pad, t2_s2s_cqcb0_to_cq_info.pad);

}

action cq_rx_eqcb_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t2_s2s_cqcb_to_eq_info_scr.cq_id, t2_s2s_cqcb_to_eq_info.cq_id);
    modify_field(t2_s2s_cqcb_to_eq_info_scr.eqcb_addr, t2_s2s_cqcb_to_eq_info.eqcb_addr);
    modify_field(t2_s2s_cqcb_to_eq_info_scr.pad, t2_s2s_cqcb_to_eq_info.pad);

}

