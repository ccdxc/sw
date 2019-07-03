/***********************************************************************/
/* nvme_req_rx.p4 */
/***********************************************************************/

#include "common_txdma_dummy.p4"

/**** table declarations ****/

#define tx_table_s0_t0 s0_t0_nvme_req_rx
#define tx_table_s0_t1 s0_t1_nvme_req_rx
#define tx_table_s0_t2 s0_t2_nvme_req_rx
#define tx_table_s0_t3 s0_t3_nvme_req_rx

#define tx_table_s1_t0 s1_t0_nvme_req_rx
#define tx_table_s1_t1 s1_t1_nvme_req_rx
#define tx_table_s1_t2 s1_t2_nvme_req_rx
#define tx_table_s1_t3 s1_t3_nvme_req_rx

#define tx_table_s2_t0 s2_t0_nvme_req_rx
#define tx_table_s2_t1 s2_t1_nvme_req_rx
#define tx_table_s2_t2 s2_t2_nvme_req_rx
#define tx_table_s2_t3 s2_t3_nvme_req_rx

#define tx_table_s3_t0 s3_t0_nvme_req_rx
#define tx_table_s3_t1 s3_t1_nvme_req_rx
#define tx_table_s3_t2 s3_t2_nvme_req_rx
#define tx_table_s3_t3 s3_t3_nvme_req_rx

#define tx_table_s4_t0 s4_t0_nvme_req_rx
#define tx_table_s4_t1 s4_t1_nvme_req_rx
#define tx_table_s4_t2 s4_t2_nvme_req_rx
#define tx_table_s4_t3 s4_t3_nvme_req_rx

#define tx_table_s5_t0 s5_t0_nvme_req_rx
#define tx_table_s5_t1 s5_t1_nvme_req_rx
#define tx_table_s5_t2 s5_t2_nvme_req_rx
#define tx_table_s5_t3 s5_t3_nvme_req_rx

#define tx_table_s6_t0 s6_t0_nvme_req_rx
#define tx_table_s6_t1 s6_t1_nvme_req_rx
#define tx_table_s6_t2 s6_t2_nvme_req_rx
#define tx_table_s6_t3 s6_t3_nvme_req_rx

#define tx_table_s7_t0 s7_t0_nvme_req_rx
#define tx_table_s7_t1 s7_t1_nvme_req_rx
#define tx_table_s7_t2 s7_t2_nvme_req_rx
#define tx_table_s7_t3 s7_t3_nvme_req_rx

#define tx_stage0_lif_params_table lif_params_req_rx
#define tx_table_s5_t4_lif_rate_limiter_table lif_rate_limiter_req_rx


/**** action declarations ****/

#define tx_table_s0_t0_action rqcb_process

#define tx_table_s1_t0_action rqe_process
#define tx_table_s1_t1_action resourcecb_process

#define tx_table_s2_t0_action pdu_hdr_process
#define tx_table_s2_t1_action pduid_fetch_process

#define tx_table_s3_t0_action c2h_data_process

#define tx_table_s4_t0_action sessprodrxcb_process

#define tx_table_s5_t0_action rqcb_writeback_process

#define tx_table_s7_t1_action rq_statscb_process

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

header_type nvme_req_rx_to_stage_rqe_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_req_rx_to_stage_pdu_hdr_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_req_rx_to_stage_c2h_data_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_req_rx_to_stage_sessprodrxcb_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_req_rx_to_stage_rqcb_writeback_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_req_rx_to_stage_rq_statscb_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_req_rx_rqcb_to_rqe_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_req_rx_rqcb_to_resourcecb_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_req_rx_rqe_to_pdu_hdr_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_req_rx_resourcecb_to_pduid_fetch_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_req_rx_pduhdr_to_c2h_data_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_req_rx_pduhdr_to_sessprodrxcb_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_req_rx_sessprodrxcb_to_rqcb_writeback_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_req_rx_rqcb_writeback_to_rq_statscb_t {
    fields {
        pad                 : 160;
    }
}

/**** scratch for D-vectors ****/

@pragma scratch_metadata
metadata rqcb_t rqcb_d;

@pragma scratch_metadata
metadata nvme_rqe_t nvme_rqe_d;

@pragma scratch_metadata
metadata resourcecb_t resourcecb_d;

@pragma scratch_metadata
metadata pdu_hdr_t pdu_hdr_d;

@pragma scratch_metadata
metadata pduid_ring_entry_t pduid_ring_entry_d;

@pragma scratch_metadata
metadata c2h_data_t c2h_data_d;

@pragma scratch_metadata
metadata sessprodrxcb_t sessprodrxcb_d;

@pragma scratch_metadata
metadata rq_statscb_t rq_statscb_d;

/**** global header unions ****/

@pragma pa_header_union ingress common_global
metadata phv_global_common_t phv_global_common;
@pragma scratch_metadata
metadata phv_global_common_t phv_global_common_scr;

/**** to stage header unions ****/

//To-Stage-0

//To-Stage-1
@pragma pa_header_union ingress to_stage_1
metadata nvme_req_rx_to_stage_rqe_info_t to_s1_info;
@pragma scratch_metadata
metadata nvme_req_rx_to_stage_rqe_info_t to_s1_info_scr;

//To-Stage-2
@pragma pa_header_union ingress to_stage_2
metadata nvme_req_rx_to_stage_pdu_hdr_info_t to_s2_info;
@pragma scratch_metadata
metadata nvme_req_rx_to_stage_pdu_hdr_info_t to_s2_info_scr;

//To-Stage-3
@pragma pa_header_union ingress to_stage_3
metadata nvme_req_rx_to_stage_c2h_data_info_t to_s3_info;
@pragma scratch_metadata
metadata nvme_req_rx_to_stage_c2h_data_info_t to_s3_info_scr;

//To-Stage-4
@pragma pa_header_union ingress to_stage_4
metadata nvme_req_rx_to_stage_sessprodrxcb_info_t to_s4_info;
@pragma scratch_metadata
metadata nvme_req_rx_to_stage_sessprodrxcb_info_t to_s4_info_scr;

//To-Stage-5
@pragma pa_header_union ingress to_stage_5
metadata nvme_req_rx_to_stage_rqcb_writeback_info_t to_s5_info;
@pragma scratch_metadata
metadata nvme_req_rx_to_stage_rqcb_writeback_info_t to_s5_info_scr;

//To-Stage-7
@pragma pa_header_union ingress to_stage_7
metadata nvme_req_rx_to_stage_rq_statscb_info_t to_s7_info;
@pragma scratch_metadata
metadata nvme_req_rx_to_stage_rq_statscb_info_t to_s7_info_scr;


/**** stage to stage header unions ****/

//Table-0
@pragma pa_header_union ingress common_t0_s2s t0_s2s_rqcb_to_rqe_info t0_s2s_rqe_to_pdu_hdr_info t0_s2s_pduhdr_to_c2h_data_info t0_s2s_pduhdr_to_sessprodrxcb_info t0_s2s_sessprodrxcb_to_rqcb_writeback_info

metadata nvme_req_rx_rqcb_to_rqe_t t0_s2s_rqcb_to_rqe_info;
@pragma scratch_metadata
metadata nvme_req_rx_rqcb_to_rqe_t t0_s2s_rqcb_to_rqe_info_scr;

metadata nvme_req_rx_rqe_to_pdu_hdr_t t0_s2s_rqe_to_pdu_hdr_info;
@pragma scratch_metadata
metadata nvme_req_rx_rqe_to_pdu_hdr_t t0_s2s_rqe_to_pdu_hdr_info_scr;

metadata nvme_req_rx_pduhdr_to_c2h_data_t t0_s2s_pduhdr_to_c2h_data_info;
@pragma scratch_metadata
metadata nvme_req_rx_pduhdr_to_c2h_data_t t0_s2s_pduhdr_to_c2h_data_info_scr;

metadata nvme_req_rx_pduhdr_to_sessprodrxcb_t t0_s2s_pduhdr_to_sessprodrxcb_info;
@pragma scratch_metadata
metadata nvme_req_rx_pduhdr_to_sessprodrxcb_t t0_s2s_pduhdr_to_sessprodrxcb_info_scr;

metadata nvme_req_rx_sessprodrxcb_to_rqcb_writeback_t t0_s2s_sessprodrxcb_to_rqcb_writeback_info;
@pragma scratch_metadata
metadata nvme_req_rx_sessprodrxcb_to_rqcb_writeback_t t0_s2s_sessprodrxcb_to_rqcb_writeback_info_scr;

//Table-1
@pragma pa_header_union ingress common_t1_s2s t1_s2s_rqcb_to_resourcecb_info t1_s2s_resourcecb_to_pduid_fetch_info t1_s2s_rqcb_writeback_to_rq_statscb_info

metadata nvme_req_rx_rqcb_to_resourcecb_t t1_s2s_rqcb_to_resourcecb_info;
@pragma scratch_metadata
metadata nvme_req_rx_rqcb_to_resourcecb_t t1_s2s_rqcb_to_resourcecb_info_scr;

metadata nvme_req_rx_resourcecb_to_pduid_fetch_t t1_s2s_resourcecb_to_pduid_fetch_info;
@pragma scratch_metadata
metadata nvme_req_rx_resourcecb_to_pduid_fetch_t t1_s2s_resourcecb_to_pduid_fetch_info_scr;

metadata nvme_req_rx_rqcb_writeback_to_rq_statscb_t t1_s2s_rqcb_writeback_to_rq_statscb_info;
@pragma scratch_metadata
metadata nvme_req_rx_rqcb_writeback_to_rq_statscb_t t1_s2s_rqcb_writeback_to_rq_statscb_info_scr;

/**** PHV Layout ****/
@pragma dont_trim
metadata sess_wqe_t sess_wqe;
@pragma dont_trim
metadata data64_t session_db;
@pragma dont_trim
metadata index16_t pduid_cindex;
@pragma dont_trim
metadata pdu_context0_t pdu_ctxt0;
@pragma dont_trim
metadata pdu_context1_t pdu_ctxt1;

@pragma pa_align 128
@pragma dont_trim
metadata dma_cmd_phv2mem_t pdu_ctxt0_dma;       //dma cmd 0
@pragma dont_trim
metadata dma_cmd_phv2mem_t pdu_ctxt1_dma;       //dma cmd 1
@pragma dont_trim
metadata dma_cmd_phv2mem_t pduid_cindex_dma;    //dma cmd 2
@pragma dont_trim
metadata dma_cmd_phv2mem_t session_wqe_dma;     //dma cmd 3
@pragma dont_trim
metadata dma_cmd_phv2mem_t session_db_dma;      //dma cmd 4


/*
 * Stage 0 table 0 action
 */
action rqcb_process (RQCB_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage

    // D-vector
    GENERATE_RQCB_D
}

action rqe_process (NVME_RQE_PARAMS) {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_info_scr.pad, to_s1_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_rqcb_to_rqe_info_scr.pad, t0_s2s_rqcb_to_rqe_info.pad);

    // D-vector
    GENERATE_NVME_RQE_D
}

action resourcecb_process (RESOURCECB_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_info_scr.pad, to_s1_info.pad);
    
    // stage to stage
    modify_field(t1_s2s_rqcb_to_resourcecb_info_scr.pad, t1_s2s_rqcb_to_resourcecb_info.pad);

    // D-vector
    GENERATE_RESOURCECB_D
}

action pdu_hdr_process (PDU_HDR_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s2_info_scr.pad, to_s2_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_rqe_to_pdu_hdr_info_scr.pad, t0_s2s_rqe_to_pdu_hdr_info.pad);

    // D-vector
    GENERATE_PDU_HDR_D
}

action sessprodrxcb_process (SESSPRODRXCB_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_info_scr.pad, to_s4_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_pduhdr_to_sessprodrxcb_info_scr.pad, t0_s2s_pduhdr_to_sessprodrxcb_info.pad);

    // D-vector
    GENERATE_SESSPRODRXCB_D
}

action pduid_fetch_process (PDUID_RING_ENTRY_PARAMS) {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s2_info_scr.pad, to_s2_info.pad);
    
    // stage to stage
    modify_field(t1_s2s_resourcecb_to_pduid_fetch_info_scr.pad, t1_s2s_resourcecb_to_pduid_fetch_info.pad);

    // D-vector
    GENERATE_PDUID_RING_ENTRY_D
}

action c2h_data_process(C2H_DATA_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s3_info_scr.pad, to_s3_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_pduhdr_to_c2h_data_info_scr.pad, t0_s2s_pduhdr_to_c2h_data_info.pad);

    // D-vector
    GENERATE_C2H_DATA_D
}

action rqcb_writeback_process (RQCB_PARAMS_NON_STG0) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s5_info_scr.pad, to_s5_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_sessprodrxcb_to_rqcb_writeback_info_scr.pad, t0_s2s_sessprodrxcb_to_rqcb_writeback_info.pad);

    // D-vector
    GENERATE_RQCB_D_NON_STG0
}

action rq_statscb_process (RQ_STATSCB_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s7_info_scr.pad, to_s7_info.pad);
    
    // stage to stage
    modify_field(t1_s2s_rqcb_writeback_to_rq_statscb_info_scr.pad, t1_s2s_rqcb_writeback_to_rq_statscb_info.pad);

    // D-vector
    GENERATE_RQ_STATSCB_D
}
