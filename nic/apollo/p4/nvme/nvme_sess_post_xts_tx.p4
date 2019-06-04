/***********************************************************************/
/* nvme_sess_post_xts_tx.p4 */
/***********************************************************************/

#include "common_txdma_dummy.p4"

/**** table declarations ****/

#define tx_table_s0_t0 s0_t0_nvme_sesspostxts_tx
#define tx_table_s0_t1 s0_t1_nvme_sesspostxts_tx
#define tx_table_s0_t2 s0_t2_nvme_sesspostxts_tx
#define tx_table_s0_t3 s0_t3_nvme_sesspostxts_tx

#define tx_table_s1_t0 s1_t0_nvme_sesspostxts_tx
#define tx_table_s1_t1 s1_t1_nvme_sesspostxts_tx
#define tx_table_s1_t2 s1_t2_nvme_sesspostxts_tx
#define tx_table_s1_t3 s1_t3_nvme_sesspostxts_tx

#define tx_table_s2_t0 s2_t0_nvme_sesspostxts_tx
#define tx_table_s2_t1 s2_t1_nvme_sesspostxts_tx
#define tx_table_s2_t2 s2_t2_nvme_sesspostxts_tx
#define tx_table_s2_t3 s2_t3_nvme_sesspostxts_tx

#define tx_table_s3_t0 s3_t0_nvme_sesspostxts_tx
#define tx_table_s3_t1 s3_t1_nvme_sesspostxts_tx
#define tx_table_s3_t2 s3_t2_nvme_sesspostxts_tx
#define tx_table_s3_t3 s3_t3_nvme_sesspostxts_tx

#define tx_table_s4_t0 s4_t0_nvme_sesspostxts_tx
#define tx_table_s4_t1 s4_t1_nvme_sesspostxts_tx
#define tx_table_s4_t2 s4_t2_nvme_sesspostxts_tx
#define tx_table_s4_t3 s4_t3_nvme_sesspostxts_tx

#define tx_table_s5_t0 s5_t0_nvme_sesspostxts_tx
#define tx_table_s5_t1 s5_t1_nvme_sesspostxts_tx
#define tx_table_s5_t2 s5_t2_nvme_sesspostxts_tx
#define tx_table_s5_t3 s5_t3_nvme_sesspostxts_tx

#define tx_table_s6_t0 s6_t0_nvme_sesspostxts_tx
#define tx_table_s6_t1 s6_t1_nvme_sesspostxts_tx
#define tx_table_s6_t2 s6_t2_nvme_sesspostxts_tx
#define tx_table_s6_t3 s6_t3_nvme_sesspostxts_tx

#define tx_table_s7_t0 s7_t0_nvme_sesspostxts_tx
#define tx_table_s7_t1 s7_t1_nvme_sesspostxts_tx
#define tx_table_s7_t2 s7_t2_nvme_sesspostxts_tx
#define tx_table_s7_t3 s7_t3_nvme_sesspostxts_tx

#define tx_stage0_lif_params_table lif_params_nvme_sesspostxts_tx
#define tx_table_s5_t4_lif_rate_limiter_table lif_rate_limiter_nvme_sesspostxts_tx

/**** action declarations ****/

#define tx_table_s0_t0_action cb_process

#define tx_table_s1_t0_action sess_wqe_process

//this stage/table should be in-sync with nvme_req_tx_sessprodcb_process
//for locked table purpose as both the programs produce into dgst q
#define tx_table_s5_t0_action sessprodcb_process

#define tx_table_s6_t0_action cb_writeback_process

#define tx_table_s7_t0_action pdu_ctxt_process

#include "common_txdma.p4"
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

header_type nvme_sesspostxts_tx_to_stage_sess_wqe_info_t {
    fields {
        session_id                       :   16;
        pad                              :  112;
    }
}

header_type nvme_sesspostxts_tx_to_stage_sessprodcb_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_sesspostxts_tx_to_stage_writeback_info_t {
    fields {
        pduid                            :  16;
        pad                              :  112;
    }
}

header_type nvme_sesspostxts_tx_to_stage_pdu_ctxt_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_sesspostxts_tx_cb_to_sess_wqe_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_sesspostxts_tx_sess_wqe_to_sessprodcb_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_sesspostxts_tx_sessprodcb_to_writeback_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_sesspostxts_tx_writeback_to_pdu_ctxt_t {
    fields {
        pad                 : 160;
    }
}

/**** scratch for D-vectors ****/

@pragma scratch_metadata
metadata sessxtstxcb_t sessxtstxcb_d;

@pragma scratch_metadata
metadata sess_wqe_t sess_wqe_d;

@pragma scratch_metadata
metadata sessprodcb_t sessprodcb_d;

@pragma scratch_metadata
metadata pdu_context0_t pdu_ctxt0_d;

/**** global header unions ****/

@pragma pa_header_union ingress common_global
metadata phv_global_common_t phv_global_common;
@pragma scratch_metadata
metadata phv_global_common_t phv_global_common_scr;

/**** to stage header unions ****/

//To-Stage-0

//To-Stage-1
@pragma pa_header_union ingress to_stage_1
metadata nvme_sesspostxts_tx_to_stage_sess_wqe_info_t to_s1_info;
@pragma scratch_metadata
metadata nvme_sesspostxts_tx_to_stage_sess_wqe_info_t to_s1_info_scr;

//To-Stage-5
@pragma pa_header_union ingress to_stage_5
metadata nvme_sesspostxts_tx_to_stage_sessprodcb_info_t to_s5_info;
@pragma scratch_metadata
metadata nvme_sesspostxts_tx_to_stage_sessprodcb_info_t to_s5_info_scr;

//To-Stage-6
@pragma pa_header_union ingress to_stage_6
metadata nvme_sesspostxts_tx_to_stage_writeback_info_t to_s6_info;
@pragma scratch_metadata
metadata nvme_sesspostxts_tx_to_stage_writeback_info_t to_s6_info_scr;

//To-Stage-7
@pragma pa_header_union ingress to_stage_7
metadata nvme_sesspostxts_tx_to_stage_pdu_ctxt_info_t to_s7_info;
@pragma scratch_metadata
metadata nvme_sesspostxts_tx_to_stage_pdu_ctxt_info_t to_s7_info_scr;


/**** stage to stage header unions ****/

//Table-0
@pragma pa_header_union ingress common_t0_s2s t0_s2s_cb_to_sess_wqe_info t0_s2s_sess_wqe_to_sessprodcb_info t0_s2s_sessprodcb_to_writeback_info t0_s2s_writeback_to_pdu_ctxt_info

metadata nvme_sesspostxts_tx_cb_to_sess_wqe_t t0_s2s_cb_to_sess_wqe_info;
@pragma scratch_metadata
metadata nvme_sesspostxts_tx_cb_to_sess_wqe_t t0_s2s_cb_to_sess_wqe_info_scr;

metadata nvme_sesspostxts_tx_sess_wqe_to_sessprodcb_t t0_s2s_sess_wqe_to_sessprodcb_info;
@pragma scratch_metadata
metadata nvme_sesspostxts_tx_sess_wqe_to_sessprodcb_t t0_s2s_sess_wqe_to_sessprodcb_info_scr;

metadata nvme_sesspostxts_tx_sessprodcb_to_writeback_t t0_s2s_sessprodcb_to_writeback_info;
@pragma scratch_metadata
metadata nvme_sesspostxts_tx_sessprodcb_to_writeback_t t0_s2s_sessprodcb_to_writeback_info_scr;

metadata nvme_sesspostxts_tx_writeback_to_pdu_ctxt_t  t0_s2s_writeback_to_pdu_ctxt_info;
@pragma scratch_metadata
metadata nvme_sesspostxts_tx_writeback_to_pdu_ctxt_t t0_s2s_writeback_to_pdu_ctxt_info_scr;

//Table-1

//Table-2

/**** PHV Layout ****/
@pragma dont_trim
metadata dgst_aol_desc_t last_ddgst_aol_desc;
@pragma dont_trim
@pragma pa_align 512
metadata ptr64_t aol0_next_ptr;
@pragma dont_trim
metadata ptr64_t aol1_next_ptr;
@pragma dont_trim
metadata ptr64_t aol2_next_ptr;
@pragma dont_trim
metadata ptr64_t aol3_next_ptr;
@pragma dont_trim
metadata ptr64_t aol4_next_ptr;
@pragma dont_trim
metadata sess_wqe_t sess_wqe;
@pragma dont_trim
metadata data64_t session_db;

@pragma pa_align 128
@pragma dont_trim
metadata dma_cmd_phv2mem_t last_ddgst_aol_desc_dma;
@pragma dont_trim
metadata dma_cmd_phv2mem_t aol0_next_ptr_dma;
@pragma dont_trim
metadata dma_cmd_phv2mem_t aol1_next_ptr_dma;
@pragma dont_trim
metadata dma_cmd_phv2mem_t aol2_next_ptr_dma;
@pragma dont_trim
metadata dma_cmd_phv2mem_t aol3_next_ptr_dma;
@pragma dont_trim
metadata dma_cmd_phv2mem_t aol4_next_ptr_dma;
@pragma dont_trim
metadata dma_cmd_phv2mem_t session_wqe_dma;         //dma cmd 0
@pragma dont_trim
metadata dma_cmd_phv2mem_t session_db_dma;          //dma cmd 1

/*
 * Stage 0 table 0 action
 */
action cb_process (SESSXTSTXCB_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage

    // D-vector
    GENERATE_SESSXTSTXCB_D
}

action sess_wqe_process (SESS_WQE_PARAMS) {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_info_scr.session_id, to_s1_info.session_id);
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

action cb_writeback_process (SESSXTSTXCB_PARAMS_NON_STG0) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s6_info_scr.pduid, to_s6_info.pduid);
    modify_field(to_s6_info_scr.pad, to_s6_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_sessprodcb_to_writeback_info_scr.pad, t0_s2s_sessprodcb_to_writeback_info.pad);

    // D-vector
    GENERATE_SESSXTSTXCB_D_NON_STG0
}

action pdu_ctxt_process (PDU_CTXT0_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s7_info_scr.pad, to_s7_info.pad);

    // stage to stage
    modify_field(t0_s2s_writeback_to_pdu_ctxt_info_scr.pad, t0_s2s_writeback_to_pdu_ctxt_info.pad);

    // D-vector
    GENERATE_PDU_CTXT0_D
}
