/***********************************************************************/
/* rdma_aq_txdma.p4 */
/***********************************************************************/

#include "../common-p4+/common_txdma_dummy.p4"

/**** table declarations ****/

#define tx_table_s0_t0 aq_tx_s0_t0
#define tx_table_s0_t1 aq_tx_s0_t1
#define tx_table_s0_t2 aq_tx_s0_t2
#define tx_table_s0_t3 aq_tx_s0_t3

#define tx_table_s1_t0 aq_tx_s1_t0
#define tx_table_s1_t1 aq_tx_s1_t1
#define tx_table_s1_t2 aq_tx_s1_t2
#define tx_table_s1_t3 aq_tx_s1_t3

#define tx_table_s2_t0 aq_tx_s2_t0
#define tx_table_s2_t1 aq_tx_s2_t1
#define tx_table_s2_t2 aq_tx_s2_t2
#define tx_table_s2_t3 aq_tx_s2_t3

#define tx_table_s3_t0 aq_tx_s3_t0
#define tx_table_s3_t1 aq_tx_s3_t1
#define tx_table_s3_t2 aq_tx_s3_t2
#define tx_table_s3_t3 aq_tx_s3_t3

#define tx_table_s4_t0 aq_tx_s4_t0
#define tx_table_s4_t1 aq_tx_s4_t1
#define tx_table_s4_t2 aq_tx_s4_t2
#define tx_table_s4_t3 aq_tx_s4_t3

#define tx_table_s5_t0 aq_tx_s5_t0
#define tx_table_s5_t1 aq_tx_s5_t1
#define tx_table_s5_t2 aq_tx_s5_t2
#define tx_table_s5_t3 aq_tx_s5_t3

#define tx_table_s6_t0 aq_tx_s6_t0
#define tx_table_s6_t1 aq_tx_s6_t1
#define tx_table_s6_t2 aq_tx_s6_t2
#define tx_table_s6_t3 aq_tx_s6_t3

#define tx_table_s7_t0 aq_tx_s7_t0
#define tx_table_s7_t1 aq_tx_s7_t1
#define tx_table_s7_t2 aq_tx_s7_t2
#define tx_table_s7_t3 aq_tx_s7_t3


#define tx_table_s0_t0_action aq_tx_aqcb_process

#define tx_table_s1_t0_action aq_tx_aqwqe_process

#define tx_table_s2_t1_action aq_tx_modify_qp_2_process

#define tx_table_s2_t0_action aq_tx_feedback_process_s3
#define tx_table_s3_t0_action aq_tx_feedback_process_s3
#define tx_table_s4_t0_action aq_tx_feedback_process_s4
#define tx_table_s5_t0_action aq_tx_feedback_process_s5

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

/**** header definitions ****/

header_type phv_global_common_t {
    fields {
        lif                              :   11;
        qid                              :   24;
        qtype                            :    3;
        cb_addr                          :   25;
        pt_base_addr_page_id             :   22;
        log_num_pt_entries               :    5;
        pad                              :   24;
    }
}

header_type aq_tx_to_stage_wqe_info_t {
    fields {
        cqcb_base_addr_hi                :   24;
        sqcb_base_addr_hi                :   24;
        rqcb_base_addr_hi                :   24;
        log_num_cq_entries               :    4;        
        pad                              :   52;
    }
}

header_type aq_tx_to_stage_wqe2_info_t {
    fields {
        ah_base_addr_page_id             :   22;
        rrq_base_addr_page_id            :   22;
        rsq_base_addr_page_id            :   22;
        sqcb_base_addr_hi                :   24;
        rqcb_base_addr_hi                :   24;
        pad                              :   14;
    }
}

header_type aq_tx_to_stage_fb_info_t {
    fields {
        cq_num                           :    24;
        pad                              :   104;
    }
}

/**** global header unions ****/

@pragma pa_header_union ingress common_global
metadata phv_global_common_t phv_global_common;
@pragma scratch_metadata
metadata phv_global_common_t phv_global_common_scr;

/**** to stage header unions ****/

//To-Stage-0

//To-Stage-1
@pragma pa_header_union ingress to_stage_1
metadata aq_tx_to_stage_wqe_info_t to_s1_info;
@pragma scratch_metadata
metadata aq_tx_to_stage_wqe_info_t to_s1_info_scr;

//To-Stage-2
@pragma pa_header_union ingress to_stage_2
metadata aq_tx_to_stage_wqe2_info_t to_s2_info;
@pragma scratch_metadata
metadata aq_tx_to_stage_wqe2_info_t to_s2_info_scr;

//To-Stage-5
@pragma pa_header_union ingress to_stage_5
metadata aq_tx_to_stage_fb_info_t to_s5_info;
@pragma scratch_metadata
metadata aq_tx_to_stage_fb_info_t to_s5_info_scr;

//To-Stage-6

//To-Stage-7

/**** stage to stage header unions ****/

//Table-0

//Table-1

//Table-2

//Table-3

/*
 * Stage 0 table 0 action
 */
action aq_tx_aqcb_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
}

action aq_tx_aqwqe_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_info_scr.cqcb_base_addr_hi, to_s1_info.cqcb_base_addr_hi);
    modify_field(to_s1_info_scr.sqcb_base_addr_hi, to_s1_info.sqcb_base_addr_hi);
    modify_field(to_s1_info_scr.rqcb_base_addr_hi, to_s1_info.rqcb_base_addr_hi);
    modify_field(to_s1_info_scr.log_num_cq_entries, to_s1_info.log_num_cq_entries);
    modify_field(to_s1_info_scr.pad, to_s1_info.pad);
    
    // stage to stage
}

action aq_tx_modify_qp_2_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s2_info_scr.ah_base_addr_page_id, to_s2_info.ah_base_addr_page_id);
    modify_field(to_s2_info_scr.rrq_base_addr_page_id, to_s2_info.rrq_base_addr_page_id);
    modify_field(to_s2_info_scr.rsq_base_addr_page_id, to_s2_info.rsq_base_addr_page_id);
    modify_field(to_s2_info_scr.sqcb_base_addr_hi, to_s2_info.sqcb_base_addr_hi);
    modify_field(to_s2_info_scr.rqcb_base_addr_hi, to_s2_info.rqcb_base_addr_hi);
    modify_field(to_s2_info_scr.pad, to_s2_info.pad);
    
    // stage to stage
}

action aq_tx_feedback_process_s3 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
}

action aq_tx_feedback_process_s4 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
}

action aq_tx_feedback_process_s5 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s5_info_scr.cq_num, to_s5_info.cq_num);
    modify_field(to_s5_info_scr.pad, to_s5_info.pad);
    
    // stage to stage
}


