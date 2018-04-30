/***********************************************************************/
/* smddc_req_txdma.p4 */
/***********************************************************************/

#include "../common-p4+/common_txdma_dummy.p4"

/**** table declarations ****/

#define tx_table_s0_t0 smddc_req_tx_s0_t0
#define tx_table_s0_t1 smddc_req_tx_s0_t1
#define tx_table_s0_t2 smddc_req_tx_s0_t2
#define tx_table_s0_t3 smddc_req_tx_s0_t3

#define tx_table_s1_t0 smddc_req_tx_s1_t0
#define tx_table_s1_t1 smddc_req_tx_s1_t1
#define tx_table_s1_t2 smddc_req_tx_s1_t2
#define tx_table_s1_t3 smddc_req_tx_s1_t3

#define tx_table_s2_t0 smddc_req_tx_s2_t0
#define tx_table_s2_t1 smddc_req_tx_s2_t1
#define tx_table_s2_t2 smddc_req_tx_s2_t2
#define tx_table_s2_t3 smddc_req_tx_s2_t3

#define tx_table_s3_t0 smddc_req_tx_s3_t0
#define tx_table_s3_t1 smddc_req_tx_s3_t1
#define tx_table_s3_t2 smddc_req_tx_s3_t2
#define tx_table_s3_t3 smddc_req_tx_s3_t3

#define tx_table_s4_t0 smddc_req_tx_s4_t0
#define tx_table_s4_t1 smddc_req_tx_s4_t1
#define tx_table_s4_t2 smddc_req_tx_s4_t2
#define tx_table_s4_t3 smddc_req_tx_s4_t3

#define tx_table_s5_t0 smddc_req_tx_s5_t0
#define tx_table_s5_t1 smddc_req_tx_s5_t1
#define tx_table_s5_t2 smddc_req_tx_s5_t2
#define tx_table_s5_t3 smddc_req_tx_s5_t3

#define tx_table_s6_t0 smddc_req_tx_s6_t0
#define tx_table_s6_t1 smddc_req_tx_s6_t1
#define tx_table_s6_t2 smddc_req_tx_s6_t2
#define tx_table_s6_t3 smddc_req_tx_s6_t3

#define tx_table_s7_t0 smddc_req_tx_s7_t0
#define tx_table_s7_t1 smddc_req_tx_s7_t1
#define tx_table_s7_t2 smddc_req_tx_s7_t2
#define tx_table_s7_t3 smddc_req_tx_s7_t3


#define tx_table_s0_t0_action smddc_req_tx_process
#define tx_table_s1_t0_action smddc_req_tx_wqe_process
#define tx_table_s2_t0_action smddc_req_tx_mr_select_process
#define tx_table_s3_t0_action smddc_req_tx_post_rdma_req_process

#include "../common-p4+/common_txdma.p4"

/**** Macros ****/

#define GENERATE_GLOBAL_K \
    modify_field(phv_global_common_scr.lif, phv_global_common.lif);\
    modify_field(phv_global_common_scr.qid, phv_global_common.qid);\
    modify_field(phv_global_common_scr.qtype, phv_global_common.qtype);\
    modify_field(phv_global_common_scr.cb_addr, phv_global_common.cb_addr);\
    modify_field(phv_global_common_scr.pad, phv_global_common.pad);\

/**** header definitions ****/

header_type phv_global_common_t {
    fields {
        lif                              :   11;
        qid                              :   24;
        qtype                            :    3;
        cb_addr                          :   25;
        pad                              :   52;
    }
}

header_type smddc_req_tx_cb_to_wqe_info_t {
    fields {
        pad                              :   160;
    }
}

header_type smddc_req_tx_wqe_to_mr_select_info_t {
    fields {
        pad                              :   160;
    }
}

header_type smddc_req_tx_mr_select_to_rdma_info_t {
    fields {
        pad                              :   160;
    }
}

header_type smddc_req_tx_to_stage_t {
    fields {
        pad                              :   128;
    }
}

/**** global header unions ****/

@pragma pa_header_union ingress common_global
metadata phv_global_common_t phv_global_common;
@pragma scratch_metadata
metadata phv_global_common_t phv_global_common_scr;

/**** to stage header unions ****/

//To-Stage-0
@pragma pa_header_union ingress to_stage_0 to_s0_to_stage

metadata smddc_req_tx_to_stage_t to_s0_to_stage;
@pragma scratch_metadata
metadata smddc_req_tx_to_stage_t to_s0_to_stage_scr;


//To-Stage-1
@pragma pa_header_union ingress to_stage_1 to_s1_to_stage

metadata smddc_req_tx_to_stage_t to_s1_to_stage;
@pragma scratch_metadata
metadata smddc_req_tx_to_stage_t to_s1_to_stage_scr;

//To-Stage-2
@pragma pa_header_union ingress to_stage_2 to_s2_to_stage

metadata smddc_req_tx_to_stage_t to_s2_to_stage;
@pragma scratch_metadata
metadata smddc_req_tx_to_stage_t to_s2_to_stage_scr;

//To-Stage-3
@pragma pa_header_union ingress to_stage_3 to_s3_to_stage

metadata smddc_req_tx_to_stage_t to_s3_to_stage;
@pragma scratch_metadata
metadata smddc_req_tx_to_stage_t to_s3_to_stage_scr;

//To-Stage-4
@pragma pa_header_union ingress to_stage_4 to_s4_to_stage

metadata smddc_req_tx_to_stage_t to_s4_to_stage;
@pragma scratch_metadata
metadata smddc_req_tx_to_stage_t to_s4_to_stage_scr;

//To-Stage-5
@pragma pa_header_union ingress to_stage_5 to_s5_to_stage

metadata smddc_req_tx_to_stage_t to_s5_to_stage;
@pragma scratch_metadata
metadata smddc_req_tx_to_stage_t to_s5_to_stage_scr;

//To-Stage-6
@pragma pa_header_union ingress to_stage_6 to_s6_to_stage

metadata smddc_req_tx_to_stage_t to_s6_to_stage;
@pragma scratch_metadata
metadata smddc_req_tx_to_stage_t to_s6_to_stage_scr;

//To-Stage-7
@pragma pa_header_union ingress to_stage_7 to_s7_to_stage

metadata smddc_req_tx_to_stage_t to_s7_to_stage;
@pragma scratch_metadata
metadata smddc_req_tx_to_stage_t to_s7_to_stage_scr;


/**** stage to stage header unions ****/

//Table-0
@pragma pa_header_union ingress common_t0_s2s t0_s2s_cb_to_wqe_info t0_s2s_wqe_to_mr_select_info t0_s2s_mr_select_to_rdma_info

metadata smddc_req_tx_cb_to_wqe_info_t t0_s2s_cb_to_wqe_info;
@pragma scratch_metadata
metadata smddc_req_tx_cb_to_wqe_info_t t0_s2s_cb_to_wqe_info_scr;

metadata smddc_req_tx_wqe_to_mr_select_info_t t0_s2s_wqe_to_mr_select_info;
@pragma scratch_metadata
metadata smddc_req_tx_wqe_to_mr_select_info_t t0_s2s_wqe_to_mr_select_info_scr;

metadata smddc_req_tx_mr_select_to_rdma_info_t t0_s2s_mr_select_to_rdma_info;
@pragma scratch_metadata
metadata smddc_req_tx_mr_select_to_rdma_info_t t0_s2s_mr_select_to_rdma_info_scr;

//Table-1

//Table-2

//Table-3

action smddc_req_tx_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
}

action smddc_req_tx_wqe_process () {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_to_stage_scr.pad, to_s1_to_stage.pad);

    // stage to stage
    modify_field(t0_s2s_cb_to_wqe_info_scr.pad, t0_s2s_cb_to_wqe_info.pad);

}

action smddc_req_tx_mr_select_process () {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s2_to_stage_scr.pad, to_s2_to_stage.pad);

    // stage to stage
    modify_field(t0_s2s_wqe_to_mr_select_info_scr.pad, t0_s2s_wqe_to_mr_select_info.pad);

}

action smddc_req_tx_post_rdma_req_process () {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s3_to_stage_scr.pad, to_s3_to_stage.pad);

    // stage to stage
    modify_field(t0_s2s_mr_select_to_rdma_info_scr.pad, t0_s2s_mr_select_to_rdma_info.pad);

}

