/***********************************************************************/
/* nvme_sess_post_xts_rx.p4 */
/***********************************************************************/

#include "common_rxdma_dummy.p4"

/**** table declarations ****/

#define rx_table_s0_t0 s0_t0_nvme_sesspostxts_rx
#define rx_table_s0_t1 s0_t1_nvme_sesspostxts_rx
#define rx_table_s0_t2 s0_t2_nvme_sesspostxts_rx
#define rx_table_s0_t3 s0_t3_nvme_sesspostxts_rx

#define rx_table_s1_t0 s1_t0_nvme_sesspostxts_rx
#define rx_table_s1_t1 s1_t1_nvme_sesspostxts_rx
#define rx_table_s1_t2 s1_t2_nvme_sesspostxts_rx
#define rx_table_s1_t3 s1_t3_nvme_sesspostxts_rx

#define rx_table_s2_t0 s2_t0_nvme_sesspostxts_rx
#define rx_table_s2_t1 s2_t1_nvme_sesspostxts_rx
#define rx_table_s2_t2 s2_t2_nvme_sesspostxts_rx
#define rx_table_s2_t3 s2_t3_nvme_sesspostxts_rx

#define rx_table_s3_t0 s3_t0_nvme_sesspostxts_rx
#define rx_table_s3_t1 s3_t1_nvme_sesspostxts_rx
#define rx_table_s3_t2 s3_t2_nvme_sesspostxts_rx
#define rx_table_s3_t3 s3_t3_nvme_sesspostxts_rx

#define rx_table_s4_t0 s4_t0_nvme_sesspostxts_rx
#define rx_table_s4_t1 s4_t1_nvme_sesspostxts_rx
#define rx_table_s4_t2 s4_t2_nvme_sesspostxts_rx
#define rx_table_s4_t3 s4_t3_nvme_sesspostxts_rx

#define rx_table_s5_t0 s5_t0_nvme_sesspostxts_rx
#define rx_table_s5_t1 s5_t1_nvme_sesspostxts_rx
#define rx_table_s5_t2 s5_t2_nvme_sesspostxts_rx
#define rx_table_s5_t3 s5_t3_nvme_sesspostxts_rx

#define rx_table_s6_t0 s6_t0_nvme_sesspostxts_rx
#define rx_table_s6_t1 s6_t1_nvme_sesspostxts_rx
#define rx_table_s6_t2 s6_t2_nvme_sesspostxts_rx
#define rx_table_s6_t3 s6_t3_nvme_sesspostxts_rx

#define rx_table_s7_t0 s7_t0_nvme_sesspostxts_rx
#define rx_table_s7_t1 s7_t1_nvme_sesspostxts_rx
#define rx_table_s7_t2 s7_t2_nvme_sesspostxts_rx
#define rx_table_s7_t3 s7_t3_nvme_sesspostxts_rx

#define rx_stage0_lif_params_table lif_params_nvme_sesspostxts_rx

/**** action declarations ****/

#define rx_table_s0_t0_action cb_process

#define rx_table_s1_t0_action sess_wqe_process

#define rx_table_s5_t0_action sessprodcb_process

#define rx_table_s6_t0_action cb_writeback_process

#include "common_rxdma.p4"
#include "nvme_common.p4"

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
        cb_addr                          :   34;
        pad                              :   56;
    }
}

header_type nvme_sesspostxts_rx_to_stage_sess_wqe_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_sesspostxts_rx_to_stage_sessprodcb_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_sesspostxts_rx_to_stage_writeback_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_sesspostxts_rx_to_stage_pdu_ctxt_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_sesspostxts_rx_cb_to_sess_wqe_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_sesspostxts_rx_sess_wqe_to_sessprodcb_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_sesspostxts_rx_sessprodcb_to_writeback_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_sesspostxts_rx_writeback_to_pdu_ctxt_t {
    fields {
        pad                 : 160;
    }
}

/**** scratch for D-vectors ****/

@pragma scratch_metadata
metadata sessxtsrxcb_t sessxtsrxcb_d;

@pragma scratch_metadata
metadata sess_wqe_t sess_wqe_d;

@pragma scratch_metadata
metadata sessprodcb_t sessprodcb_d;

/**** global header unions ****/

@pragma pa_header_union ingress common_global
metadata phv_global_common_t phv_global_common;
@pragma scratch_metadata
metadata phv_global_common_t phv_global_common_scr;

/**** to stage header unions ****/

//To-Stage-0

//To-Stage-1
@pragma pa_header_union ingress to_stage_1
metadata nvme_sesspostxts_rx_to_stage_sess_wqe_info_t to_s1_info;
@pragma scratch_metadata
metadata nvme_sesspostxts_rx_to_stage_sess_wqe_info_t to_s1_info_scr;

//To-Stage-5
@pragma pa_header_union ingress to_stage_5
metadata nvme_sesspostxts_rx_to_stage_sessprodcb_info_t to_s5_info;
@pragma scratch_metadata
metadata nvme_sesspostxts_rx_to_stage_sessprodcb_info_t to_s5_info_scr;

//To-Stage-6
@pragma pa_header_union ingress to_stage_6
metadata nvme_sesspostxts_rx_to_stage_writeback_info_t to_s6_info;
@pragma scratch_metadata
metadata nvme_sesspostxts_rx_to_stage_writeback_info_t to_s6_info_scr;

//To-Stage-7

/**** stage to stage header unions ****/

//Table-0
@pragma pa_header_union ingress common_t0_s2s t0_s2s_cb_to_sess_wqe_info t0_s2s_sess_wqe_to_sessprodcb_info t0_s2s_sessprodcb_to_writeback_info

metadata nvme_sesspostxts_rx_cb_to_sess_wqe_t t0_s2s_cb_to_sess_wqe_info;
@pragma scratch_metadata
metadata nvme_sesspostxts_rx_cb_to_sess_wqe_t t0_s2s_cb_to_sess_wqe_info_scr;

metadata nvme_sesspostxts_rx_sess_wqe_to_sessprodcb_t t0_s2s_sess_wqe_to_sessprodcb_info;
@pragma scratch_metadata
metadata nvme_sesspostxts_rx_sess_wqe_to_sessprodcb_t t0_s2s_sess_wqe_to_sessprodcb_info_scr;

metadata nvme_sesspostxts_rx_sessprodcb_to_writeback_t t0_s2s_sessprodcb_to_writeback_info;
@pragma scratch_metadata
metadata nvme_sesspostxts_rx_sessprodcb_to_writeback_t t0_s2s_sessprodcb_to_writeback_info_scr;

//Table-1

//Table-2

/**** PHV Layout ****/
@pragma dont_trim
metadata sess_cqe_t sess_cqe;
@pragma dont_trim
metadata data64_t sess_cq_db;

@pragma pa_align 128
@pragma dont_trim
metadata dma_cmd_phv2mem_t sess_cqe_dma;         //dma cmd 0
@pragma dont_trim
metadata dma_cmd_phv2mem_t sess_cq_db_dma;          //dma cmd 1

/*
 * Stage 0 table 0 action
 */
action cb_process (SESSXTSRXCB_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage

    // D-vector
    GENERATE_SESSXTSRXCB_D
}

action sess_wqe_process (SESS_WQE_PARAMS) {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_info_scr.pad, to_s1_info.pad);

    // stage to stage
    modify_field(t0_s2s_cb_to_sess_wqe_info_scr.pad, t0_s2s_cb_to_sess_wqe_info.pad);

    // D-vector
    GENERATE_SESS_WQE_D
}

action sessprodcb_process (SESSPRODCB_PARAMS) {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s5_info_scr.pad, to_s5_info.pad);

    // stage to stage
    modify_field(t0_s2s_sess_wqe_to_sessprodcb_info_scr.pad, t0_s2s_sess_wqe_to_sessprodcb_info.pad);

    // D-vector
    GENERATE_SESSPRODCB_D
}

action cb_writeback_process (SESSXTSRXCB_PARAMS_NON_STG0) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s6_info_scr.pad, to_s6_info.pad);

    // stage to stage
    modify_field(t0_s2s_sessprodcb_to_writeback_info_scr.pad, t0_s2s_sessprodcb_to_writeback_info.pad);

    // D-vector
    GENERATE_SESSXTSRXCB_D_NON_STG0
}
