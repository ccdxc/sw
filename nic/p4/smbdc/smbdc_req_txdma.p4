/***********************************************************************/
/* smbdc_req_txdma.p4 */
/***********************************************************************/

#include "../common-p4+/common_txdma_dummy.p4"

/**** table declarations ****/

#define tx_table_s0_t0 smbdc_req_tx_s0_t0
#define tx_table_s0_t1 smbdc_req_tx_s0_t1
#define tx_table_s0_t2 smbdc_req_tx_s0_t2
#define tx_table_s0_t3 smbdc_req_tx_s0_t3

#define tx_table_s1_t0 smbdc_req_tx_s1_t0
#define tx_table_s1_t1 smbdc_req_tx_s1_t1
#define tx_table_s1_t2 smbdc_req_tx_s1_t2
#define tx_table_s1_t3 smbdc_req_tx_s1_t3

#define tx_table_s2_t0 smbdc_req_tx_s2_t0
#define tx_table_s2_t1 smbdc_req_tx_s2_t1
#define tx_table_s2_t2 smbdc_req_tx_s2_t2
#define tx_table_s2_t3 smbdc_req_tx_s2_t3

#define tx_table_s3_t0 smbdc_req_tx_s3_t0
#define tx_table_s3_t1 smbdc_req_tx_s3_t1
#define tx_table_s3_t2 smbdc_req_tx_s3_t2
#define tx_table_s3_t3 smbdc_req_tx_s3_t3

#define tx_table_s4_t0 smbdc_req_tx_s4_t0
#define tx_table_s4_t1 smbdc_req_tx_s4_t1
#define tx_table_s4_t2 smbdc_req_tx_s4_t2
#define tx_table_s4_t3 smbdc_req_tx_s4_t3

#define tx_table_s5_t0 smbdc_req_tx_s5_t0
#define tx_table_s5_t1 smbdc_req_tx_s5_t1
#define tx_table_s5_t2 smbdc_req_tx_s5_t2
#define tx_table_s5_t3 smbdc_req_tx_s5_t3

#define tx_table_s6_t0 smbdc_req_tx_s6_t0
#define tx_table_s6_t1 smbdc_req_tx_s6_t1
#define tx_table_s6_t2 smbdc_req_tx_s6_t2
#define tx_table_s6_t3 smbdc_req_tx_s6_t3

#define tx_table_s7_t0 smbdc_req_tx_s7_t0
#define tx_table_s7_t1 smbdc_req_tx_s7_t1
#define tx_table_s7_t2 smbdc_req_tx_s7_t2
#define tx_table_s7_t3 smbdc_req_tx_s7_t3


#define tx_table_s0_t0_action smbdc_req_tx_process
#define tx_table_s1_t0_action smbdc_req_tx_wqe_process
#define tx_table_s1_t0_action1 smbdc_req_tx_rdma_cqe_process
#define tx_table_s2_t0_action smbdc_req_tx_mr_select_process
#define tx_table_s2_t0_action1 smbdc_req_tx_wqe_context_process
#define tx_table_s3_t0_action smbdc_req_tx_post_rdma_req_process

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

header_type smbdc_req_tx_sqcb_to_wqe_info_t {
    fields {
        wqe_addr                         :   64;
        pad                              :   96;
    }
}

header_type smbdc_req_tx_wqe_to_mr_select_info_t {
    fields {
        num_mrs                          :     8;
        pad                              :   152;
    }
}

header_type smbdc_req_tx_mr_select_to_rdma_info_t {
    fields {
        num_mrs                          :     8;
        dma_cmd_start_index              :     5;
        pad                              :   147;
    }
}

header_type smbdc_req_tx_rdma_cqe_info_t {
    fields {
        wqe_context_addr                 :   34;
        pad                              :   126;
    }
}

header_type smbdc_req_tx_rdma_cqe_to_wqe_context_info_t {
    fields {
        msn                              :   24;
        pad                              :   136;
    }
}


header_type smbdc_req_tx_to_stage_t {
    fields {
        pad                              :   128;
    }
}

header_type smbdc_req_tx_to_stage_s2_t {
    fields {
        cqcb_addr                        :   34;
        pad                              :   94;
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

metadata smbdc_req_tx_to_stage_t to_s0_to_stage;
@pragma scratch_metadata
metadata smbdc_req_tx_to_stage_t to_s0_to_stage_scr;


//To-Stage-1
@pragma pa_header_union ingress to_stage_1 to_s1_to_stage

metadata smbdc_req_tx_to_stage_t to_s1_to_stage;
@pragma scratch_metadata
metadata smbdc_req_tx_to_stage_t to_s1_to_stage_scr;

//To-Stage-2
@pragma pa_header_union ingress to_stage_2 to_s2_to_stage

metadata smbdc_req_tx_to_stage_s2_t to_s2_to_stage;
@pragma scratch_metadata
metadata smbdc_req_tx_to_stage_s2_t to_s2_to_stage_scr;

//To-Stage-3
@pragma pa_header_union ingress to_stage_3 to_s3_to_stage

metadata smbdc_req_tx_to_stage_t to_s3_to_stage;
@pragma scratch_metadata
metadata smbdc_req_tx_to_stage_t to_s3_to_stage_scr;

//To-Stage-4
@pragma pa_header_union ingress to_stage_4 to_s4_to_stage

metadata smbdc_req_tx_to_stage_t to_s4_to_stage;
@pragma scratch_metadata
metadata smbdc_req_tx_to_stage_t to_s4_to_stage_scr;

//To-Stage-5
@pragma pa_header_union ingress to_stage_5 to_s5_to_stage

metadata smbdc_req_tx_to_stage_t to_s5_to_stage;
@pragma scratch_metadata
metadata smbdc_req_tx_to_stage_t to_s5_to_stage_scr;

//To-Stage-6
@pragma pa_header_union ingress to_stage_6 to_s6_to_stage

metadata smbdc_req_tx_to_stage_t to_s6_to_stage;
@pragma scratch_metadata
metadata smbdc_req_tx_to_stage_t to_s6_to_stage_scr;

//To-Stage-7
@pragma pa_header_union ingress to_stage_7 to_s7_to_stage

metadata smbdc_req_tx_to_stage_t to_s7_to_stage;
@pragma scratch_metadata
metadata smbdc_req_tx_to_stage_t to_s7_to_stage_scr;


/**** stage to stage header unions ****/

//Table-0
@pragma pa_header_union ingress common_t0_s2s t0_s2s_sqcb_to_wqe_info t0_s2s_wqe_to_mr_select_info t0_s2s_mr_select_to_rdma_info t0_s2s_rdma_cqe_info t0_s2s_rdma_cqe_to_wqe_context_info

metadata smbdc_req_tx_sqcb_to_wqe_info_t t0_s2s_sqcb_to_wqe_info;
@pragma scratch_metadata
metadata smbdc_req_tx_sqcb_to_wqe_info_t t0_s2s_sqcb_to_wqe_info_scr;

metadata smbdc_req_tx_rdma_cqe_info_t t0_s2s_rdma_cqe_info;
@pragma scratch_metadata
metadata smbdc_req_tx_rdma_cqe_info_t t0_s2s_rdma_cqe_info_scr;

metadata smbdc_req_tx_rdma_cqe_to_wqe_context_info_t t0_s2s_rdma_cqe_to_wqe_context_info;
@pragma scratch_metadata
metadata smbdc_req_tx_rdma_cqe_to_wqe_context_info_t t0_s2s_rdma_cqe_to_wqe_context_info_scr;

metadata smbdc_req_tx_wqe_to_mr_select_info_t t0_s2s_wqe_to_mr_select_info;
@pragma scratch_metadata
metadata smbdc_req_tx_wqe_to_mr_select_info_t t0_s2s_wqe_to_mr_select_info_scr;

metadata smbdc_req_tx_mr_select_to_rdma_info_t t0_s2s_mr_select_to_rdma_info;
@pragma scratch_metadata
metadata smbdc_req_tx_mr_select_to_rdma_info_t t0_s2s_mr_select_to_rdma_info_scr;

//Table-1

//Table-2

//Table-3

action smbdc_req_tx_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
}

action smbdc_req_tx_wqe_process () {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_to_stage_scr.pad, to_s1_to_stage.pad);

    // stage to stage
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.pad, t0_s2s_sqcb_to_wqe_info.pad);

}

action smbdc_req_tx_mr_select_process () {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s2_to_stage_scr.pad, to_s2_to_stage.pad);

    // stage to stage
    modify_field(t0_s2s_wqe_to_mr_select_info_scr.num_mrs, t0_s2s_wqe_to_mr_select_info.num_mrs);
    modify_field(t0_s2s_wqe_to_mr_select_info_scr.pad, t0_s2s_wqe_to_mr_select_info.pad);

}

action smbdc_req_tx_post_rdma_req_process () {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s3_to_stage_scr.pad, to_s3_to_stage.pad);

    // stage to stage
    modify_field(t0_s2s_mr_select_to_rdma_info_scr.num_mrs, t0_s2s_mr_select_to_rdma_info.num_mrs);
    modify_field(t0_s2s_mr_select_to_rdma_info_scr.dma_cmd_start_index, t0_s2s_mr_select_to_rdma_info.dma_cmd_start_index);
    modify_field(t0_s2s_mr_select_to_rdma_info_scr.pad, t0_s2s_mr_select_to_rdma_info.pad);

}

action smbdc_req_tx_rdma_cqe_process () {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_to_stage_scr.pad, to_s1_to_stage.pad);

    // stage to stage
    modify_field(t0_s2s_rdma_cqe_info_scr.wqe_context_addr, t0_s2s_rdma_cqe_info.wqe_context_addr);
    modify_field(t0_s2s_rdma_cqe_info_scr.pad, t0_s2s_rdma_cqe_info.pad);
}

action smbdc_req_tx_wqe_context_process () {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s2_to_stage_scr.pad, to_s2_to_stage.pad);
    modify_field(to_s2_to_stage_scr.cqcb_addr, to_s2_to_stage.cqcb_addr);

    // stage to stage
    modify_field(t0_s2s_rdma_cqe_to_wqe_context_info_scr.msn, t0_s2s_rdma_cqe_to_wqe_context_info.msn);
    modify_field(t0_s2s_rdma_cqe_to_wqe_context_info_scr.pad, t0_s2s_rdma_cqe_to_wqe_context_info.pad);
}
