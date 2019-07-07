/***********************************************************************/
/* nvme_sess_rsrc_free_rx.p4 */
/***********************************************************************/

#include "common_txdma_dummy.p4"

/**** table declarations ****/

#define tx_table_s0_t0 s0_t0_nvme_sessrsrcfree_rx
#define tx_table_s0_t1 s0_t1_nvme_sessrsrcfree_rx
#define tx_table_s0_t2 s0_t2_nvme_sessrsrcfree_rx
#define tx_table_s0_t3 s0_t3_nvme_sessrsrcfree_rx

#define tx_table_s1_t0 s1_t0_nvme_sessrsrcfree_rx
#define tx_table_s1_t1 s1_t1_nvme_sessrsrcfree_rx
#define tx_table_s1_t2 s1_t2_nvme_sessrsrcfree_rx
#define tx_table_s1_t3 s1_t3_nvme_sessrsrcfree_rx

#define tx_table_s2_t0 s2_t0_nvme_sessrsrcfree_rx
#define tx_table_s2_t1 s2_t1_nvme_sessrsrcfree_rx
#define tx_table_s2_t2 s2_t2_nvme_sessrsrcfree_rx
#define tx_table_s2_t3 s2_t3_nvme_sessrsrcfree_rx

#define tx_table_s3_t0 s3_t0_nvme_sessrsrcfree_rx
#define tx_table_s3_t1 s3_t1_nvme_sessrsrcfree_rx
#define tx_table_s3_t2 s3_t2_nvme_sessrsrcfree_rx
#define tx_table_s3_t3 s3_t3_nvme_sessrsrcfree_rx

#define tx_table_s4_t0 s4_t0_nvme_sessrsrcfree_rx
#define tx_table_s4_t1 s4_t1_nvme_sessrsrcfree_rx
#define tx_table_s4_t2 s4_t2_nvme_sessrsrcfree_rx
#define tx_table_s4_t3 s4_t3_nvme_sessrsrcfree_rx

#define tx_table_s5_t0 s5_t0_nvme_sessrsrcfree_rx
#define tx_table_s5_t1 s5_t1_nvme_sessrsrcfree_rx
#define tx_table_s5_t2 s5_t2_nvme_sessrsrcfree_rx
#define tx_table_s5_t3 s5_t3_nvme_sessrsrcfree_rx

#define tx_table_s6_t0 s6_t0_nvme_sessrsrcfree_rx
#define tx_table_s6_t1 s6_t1_nvme_sessrsrcfree_rx
#define tx_table_s6_t2 s6_t2_nvme_sessrsrcfree_rx
#define tx_table_s6_t3 s6_t3_nvme_sessrsrcfree_rx

#define tx_table_s7_t0 s7_t0_nvme_sessrsrcfree_rx
#define tx_table_s7_t1 s7_t1_nvme_sessrsrcfree_rx
#define tx_table_s7_t2 s7_t2_nvme_sessrsrcfree_rx
#define tx_table_s7_t3 s7_t3_nvme_sessrsrcfree_rx

#define tx_stage0_lif_params_table lif_params_req_rx
#define tx_table_s5_t4_lif_rate_limiter_table lif_rate_limiter_req_rx


/**** action declarations ****/

#define tx_table_s0_t0_action rfcb_process

#define tx_table_s1_t0_action rfwqe_process
#define tx_table_s1_t1_action cmd_ctxt_process

#define tx_table_s2_t0_action pdu_ctxt_fetch_pages_1_process
#define tx_table_s2_t1_action pdu_ctxt_fetch_pages_2_process
#define tx_table_s2_t2_action cqcb_process

#define tx_table_s3_t0_action decr_refcnt_first_page_process
#define tx_table_s3_t1_action decr_refcnt_last_page_process
#define tx_table_s3_t2_action pdu_ctxt_process

#define tx_table_s4_t0_action nmdpr_resourcecb_process

#define tx_table_s5_t0_action resourcecb_process

#define tx_table_s6_t0_action rfcb_writeback_process

#define tx_table_s7_t1_action rf_statscb_process

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

header_type nvme_sessrsrcfree_rx_to_stage_rfwqe_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_sessrsrcfree_rx_to_stage_pdu_ctxt_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_sessrsrcfree_rx_to_stage_decr_refcnt_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_sessrsrcfree_rx_to_nmdpr_resourcecb_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_sessrsrcfree_rx_to_resourcecb_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_sessrsrcfree_rx_to_stage_rfcb_writeback_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_sessrsrcfree_rx_to_stage_rf_statscb_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_sessrsrcfree_rx_rfcb_to_rfwqe_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_sessrsrcfree_rx_rfcb_to_cmdctxt_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_sessrsrcfree_rx_rfwqe_to_pductxt_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_sessrsrcfree_rx_pductxt_to_decr_refcnt_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_sessrsrcfree_rx_decr_refcnt_to_nmdpr_resourcecb_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_sessrsrcfree_rx_nmdpr_resourcecb_to_resourcecb_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_sessrsrcfree_rx_resourcecb_to_rfcb_writeback_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_sessrsrcfree_rx_rfcb_writeback_to_rf_statscb_t {
    fields {
        pad                 : 160;
    }
}

/**** scratch for D-vectors ****/

@pragma scratch_metadata
metadata rfcb_t rfcb_d;

@pragma scratch_metadata
metadata sess_rfwqe_t sess_rfwqe_d;

@pragma scratch_metadata
metadata pdu_ctxt_page_ptrs_cb_t pdu_ctxt_page_ptrs_cb_d;

@pragma scratch_metadata
metadata page_metadata_cb_t page_metadata_cb_d;

@pragma scratch_metadata
metadata nmdpr_resourcecb_t nmdpr_resourcecb_d;

@pragma scratch_metadata
metadata resourcecb_t resourcecb_d;

@pragma scratch_metadata
metadata rf_statscb_t rf_statscb_d;

@pragma scratch_metadata
metadata cqcb_t cqcb_d;

@pragma scratch_metadata
metadata cmd_context_t cmd_ctxt_d;

@pragma scratch_metadata
metadata pdu_context1_t pdu_ctxt1_d;


/**** global header unions ****/

@pragma pa_header_union ingress common_global
metadata phv_global_common_t phv_global_common;
@pragma scratch_metadata
metadata phv_global_common_t phv_global_common_scr;

/**** to stage header unions ****/

//To-Stage-0

//To-Stage-1
@pragma pa_header_union ingress to_stage_1
metadata nvme_sessrsrcfree_rx_to_stage_rfwqe_info_t to_s1_info;
@pragma scratch_metadata
metadata nvme_sessrsrcfree_rx_to_stage_rfwqe_info_t to_s1_info_scr;

//To-Stage-2
@pragma pa_header_union ingress to_stage_2
metadata nvme_sessrsrcfree_rx_to_stage_pdu_ctxt_info_t to_s2_info;
@pragma scratch_metadata
metadata nvme_sessrsrcfree_rx_to_stage_pdu_ctxt_info_t to_s2_info_scr;

//To-Stage-3
@pragma pa_header_union ingress to_stage_3
metadata nvme_sessrsrcfree_rx_to_stage_decr_refcnt_info_t to_s3_info;
@pragma scratch_metadata
metadata nvme_sessrsrcfree_rx_to_stage_decr_refcnt_info_t to_s3_info_scr;

//To-Stage-4
@pragma pa_header_union ingress to_stage_4
metadata nvme_sessrsrcfree_rx_to_nmdpr_resourcecb_info_t to_s4_info;
@pragma scratch_metadata
metadata nvme_sessrsrcfree_rx_to_nmdpr_resourcecb_info_t to_s4_info_scr;

//To-Stage-5
@pragma pa_header_union ingress to_stage_5
metadata nvme_sessrsrcfree_rx_to_resourcecb_info_t to_s5_info;
@pragma scratch_metadata
metadata nvme_sessrsrcfree_rx_to_resourcecb_info_t to_s5_info_scr;

//To-Stage-6
@pragma pa_header_union ingress to_stage_6
metadata nvme_sessrsrcfree_rx_to_stage_rfcb_writeback_info_t to_s6_info;
@pragma scratch_metadata
metadata nvme_sessrsrcfree_rx_to_stage_rfcb_writeback_info_t to_s6_info_scr;

//To-Stage-7
@pragma pa_header_union ingress to_stage_7
metadata nvme_sessrsrcfree_rx_to_stage_rf_statscb_info_t to_s7_info;
@pragma scratch_metadata
metadata nvme_sessrsrcfree_rx_to_stage_rf_statscb_info_t to_s7_info_scr;


/**** stage to stage header unions ****/

//Table-0
@pragma pa_header_union ingress common_t0_s2s t0_s2s_rfcb_to_rfwqe_info t0_s2s_rfwqe_to_pductxt_info t0_s2s_pductxt_to_decr_refcnt_info t0_s2s_decr_refcnt_to_nmdpr_resourcecb_info t0_s2s_nmdpr_resourcecb_to_resourcecb_info t0_s2s_resourcecb_to_rfcb_writeback_info 

metadata nvme_sessrsrcfree_rx_rfcb_to_rfwqe_t t0_s2s_rfcb_to_rfwqe_info;
@pragma scratch_metadata
metadata nvme_sessrsrcfree_rx_rfcb_to_rfwqe_t t0_s2s_rfcb_to_rfwqe_info_scr;

metadata nvme_sessrsrcfree_rx_rfwqe_to_pductxt_t t0_s2s_rfwqe_to_pductxt_info;
@pragma scratch_metadata
metadata nvme_sessrsrcfree_rx_rfwqe_to_pductxt_t t0_s2s_rfwqe_to_pductxt_info_scr;

metadata nvme_sessrsrcfree_rx_pductxt_to_decr_refcnt_t t0_s2s_pductxt_to_decr_refcnt_info;
@pragma scratch_metadata
metadata nvme_sessrsrcfree_rx_pductxt_to_decr_refcnt_t t0_s2s_pductxt_to_decr_refcnt_info_scr;

metadata nvme_sessrsrcfree_rx_decr_refcnt_to_nmdpr_resourcecb_t t0_s2s_decr_refcnt_to_nmdpr_resourcecb_info;
@pragma scratch_metadata
metadata nvme_sessrsrcfree_rx_decr_refcnt_to_nmdpr_resourcecb_t t0_s2s_decr_refcnt_to_nmdpr_resourcecb_info_scr;

metadata nvme_sessrsrcfree_rx_nmdpr_resourcecb_to_resourcecb_t t0_s2s_nmdpr_resourcecb_to_resourcecb_info;
@pragma scratch_metadata
metadata nvme_sessrsrcfree_rx_nmdpr_resourcecb_to_resourcecb_t t0_s2s_nmdpr_resourcecb_to_resourcecb_info_scr;

metadata nvme_sessrsrcfree_rx_resourcecb_to_rfcb_writeback_t t0_s2s_resourcecb_to_rfcb_writeback_info;
@pragma scratch_metadata
metadata nvme_sessrsrcfree_rx_resourcecb_to_rfcb_writeback_t t0_s2s_resourcecb_to_rfcb_writeback_info_scr;


//Table-1
@pragma pa_header_union ingress common_t1_s2s t1_s2s_rfwqe_to_pductxt_info t1_s2s_pductxt_to_decr_refcnt_info t1_s2s_rfcb_writeback_to_rf_statscb_info

metadata nvme_sessrsrcfree_rx_rfwqe_to_pductxt_t t1_s2s_rfwqe_to_pductxt_info;
@pragma scratch_metadata
metadata nvme_sessrsrcfree_rx_rfwqe_to_pductxt_t t1_s2s_rfwqe_to_pductxt_info_scr;

metadata nvme_sessrsrcfree_rx_pductxt_to_decr_refcnt_t t1_s2s_pductxt_to_decr_refcnt_info;
@pragma scratch_metadata
metadata nvme_sessrsrcfree_rx_pductxt_to_decr_refcnt_t t1_s2s_pductxt_to_decr_refcnt_info_scr;

metadata nvme_sessrsrcfree_rx_rfcb_writeback_to_rf_statscb_t t1_s2s_rfcb_writeback_to_rf_statscb_info;
@pragma scratch_metadata
metadata nvme_sessrsrcfree_rx_rfcb_writeback_to_rf_statscb_t t1_s2s_rfcb_writeback_to_rf_statscb_info_scr;


/**** PHV Layout ****/
@pragma dont_trim
metadata index16_t  cmdid;
@pragma dont_trim
metadata index16_t  pduid;
@pragma dont_trim
metadata index16_t  cq_pindex;
@pragma dont_trim
metadata index16_t  pduid_pindex;
@pragma dont_trim
metadata index16_t  cmdid_pindex;
@pragma dont_trim
metadata nvme_cqe_t nvme_cqe;

@pragma pa_align 128
@pragma dont_trim
metadata dma_cmd_mem2mem_t free_pages_src_dma1;         //dma cmd 0
@pragma dont_trim
metadata dma_cmd_mem2mem_t free_pages_dst_dma1;         //dma cmd 1
@pragma dont_trim
metadata dma_cmd_mem2mem_t free_pages_src_dma2;         //dma cmd 2
@pragma dont_trim
metadata dma_cmd_mem2mem_t free_pages_dst_dma2;         //dma cmd 3
@pragma dont_trim
metadata dma_cmd_phv2mem_t free_pduid_dma;              //dma cmd 4
@pragma dont_trim
metadata dma_cmd_phv2mem_t pduid_pindex_dma;            //dma cmd 5
@pragma dont_trim
metadata dma_cmd_phv2mem_t free_cmdid_dma;              //dma cmd 6
@pragma dont_trim
metadata dma_cmd_phv2mem_t cmdid_pindex_dma;            //dma cmd 7
@pragma dont_trim
metadata dma_cmd_mem2mem_t cqe_sq_head_ptr_src_dma;     //dma cmd 8
@pragma dont_trim
metadata dma_cmd_mem2mem_t cqe_sq_head_ptr_dst_dma;     //dma cmd 9
@pragma dont_trim
metadata dma_cmd_phv2mem_t cqe_pre_sq_head_ptr_dma;     //dma cmd 10
@pragma dont_trim
metadata dma_cmd_phv2mem_t cqe_post_sq_head_ptr_dma;    //dma cmd 11


/*
 * Stage 0 table 0 action
 */
action rfcb_process (RFCB_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage

    // D-vector
    GENERATE_RFCB_D
}

action rfwqe_process (SESS_RFWQE_PARAMS) {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_info_scr.pad, to_s1_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_rfcb_to_rfwqe_info_scr.pad, t0_s2s_rfcb_to_rfwqe_info.pad);

    // D-vector
    GENERATE_SESS_RFWQE_D
}

action cmd_ctxt_process (CMD_CTXT_PARAMS) {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_info_scr.pad, to_s1_info.pad);

    // stage to stage

    // D-vector
    GENERATE_CMD_CTXT_D
}

action pdu_ctxt_fetch_pages_1_process (PDU_CTXT_PAGE_PTRS_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s2_info_scr.pad, to_s2_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_rfwqe_to_pductxt_info_scr.pad, t0_s2s_rfwqe_to_pductxt_info.pad);

    // D-vector
    GENERATE_PDU_CTXT_PAGE_PTRS_D
}

action pdu_ctxt_fetch_pages_2_process (PDU_CTXT_PAGE_PTRS_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s2_info_scr.pad, to_s2_info.pad);
    
    // stage to stage
    modify_field(t1_s2s_rfwqe_to_pductxt_info_scr.pad, t1_s2s_rfwqe_to_pductxt_info.pad);

    // D-vector
    GENERATE_PDU_CTXT_PAGE_PTRS_D
}

action cqcb_process (CQCB_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s2_info_scr.pad, to_s2_info.pad);

    // stage to stage

    // D-vector
    GENERATE_CQCB_D
}

action decr_refcnt_first_page_process (PAGE_METADATA_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s3_info_scr.pad, to_s3_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_pductxt_to_decr_refcnt_info_scr.pad, t0_s2s_pductxt_to_decr_refcnt_info.pad);

    // D-vector
    GENERATE_PAGE_METADATA_D
}

action decr_refcnt_last_page_process (PAGE_METADATA_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s3_info_scr.pad, to_s3_info.pad);
    
    // stage to stage
    modify_field(t1_s2s_pductxt_to_decr_refcnt_info_scr.pad, t1_s2s_pductxt_to_decr_refcnt_info.pad);

    // D-vector
    GENERATE_PAGE_METADATA_D
}

action pdu_ctxt_process (PDU_CTXT1_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s3_info_scr.pad, to_s3_info.pad);

    // stage to stage

    // D-vector
    GENERATE_PDU_CTXT1_D
}

action nmdpr_resourcecb_process(NMDPR_RESOURCECB_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_info_scr.pad, to_s4_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_decr_refcnt_to_nmdpr_resourcecb_info_scr.pad, t0_s2s_decr_refcnt_to_nmdpr_resourcecb_info.pad);

    // D-vector
    GENERATE_NMDPR_RESOURCECB_D
}


action resourcecb_process(RESOURCECB_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s5_info_scr.pad, to_s5_info.pad);

    // stage to stage
    modify_field(t0_s2s_nmdpr_resourcecb_to_resourcecb_info_scr.pad, t0_s2s_nmdpr_resourcecb_to_resourcecb_info.pad);

    // D-vector
    GENERATE_RESOURCECB_D
}

action rfcb_writeback_process (RFCB_PARAMS_NON_STG0) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s6_info_scr.pad, to_s6_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_resourcecb_to_rfcb_writeback_info_scr.pad, t0_s2s_resourcecb_to_rfcb_writeback_info.pad);

    // D-vector
    GENERATE_RFCB_D_NON_STG0
}

action rf_statscb_process (RF_STATSCB_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s7_info_scr.pad, to_s7_info.pad);
    
    // stage to stage
    modify_field(t1_s2s_rfcb_writeback_to_rf_statscb_info_scr.pad, t1_s2s_rfcb_writeback_to_rf_statscb_info.pad);

    // D-vector
    GENERATE_RF_STATSCB_D
}
