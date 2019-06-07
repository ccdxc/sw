/***********************************************************************/
/* nvme_sess_post_xts_tx.p4 */
/***********************************************************************/

#include "common_txdma_dummy.p4"

/**** table declarations ****/

#define tx_table_s0_t0 s0_t0_nvme_sesspostdgst_tx
#define tx_table_s0_t1 s0_t1_nvme_sesspostdgst_tx
#define tx_table_s0_t2 s0_t2_nvme_sesspostdgst_tx
#define tx_table_s0_t3 s0_t3_nvme_sesspostdgst_tx

#define tx_table_s1_t0 s1_t0_nvme_sesspostdgst_tx
#define tx_table_s1_t1 s1_t1_nvme_sesspostdgst_tx
#define tx_table_s1_t2 s1_t2_nvme_sesspostdgst_tx
#define tx_table_s1_t3 s1_t3_nvme_sesspostdgst_tx

#define tx_table_s2_t0 s2_t0_nvme_sesspostdgst_tx
#define tx_table_s2_t1 s2_t1_nvme_sesspostdgst_tx
#define tx_table_s2_t2 s2_t2_nvme_sesspostdgst_tx
#define tx_table_s2_t3 s2_t3_nvme_sesspostdgst_tx

#define tx_table_s3_t0 s3_t0_nvme_sesspostdgst_tx
#define tx_table_s3_t1 s3_t1_nvme_sesspostdgst_tx
#define tx_table_s3_t2 s3_t2_nvme_sesspostdgst_tx
#define tx_table_s3_t3 s3_t3_nvme_sesspostdgst_tx

#define tx_table_s4_t0 s4_t0_nvme_sesspostdgst_tx
#define tx_table_s4_t1 s4_t1_nvme_sesspostdgst_tx
#define tx_table_s4_t2 s4_t2_nvme_sesspostdgst_tx
#define tx_table_s4_t3 s4_t3_nvme_sesspostdgst_tx

#define tx_table_s5_t0 s5_t0_nvme_sesspostdgst_tx
#define tx_table_s5_t1 s5_t1_nvme_sesspostdgst_tx
#define tx_table_s5_t2 s5_t2_nvme_sesspostdgst_tx
#define tx_table_s5_t3 s5_t3_nvme_sesspostdgst_tx

#define tx_table_s6_t0 s6_t0_nvme_sesspostdgst_tx
#define tx_table_s6_t1 s6_t1_nvme_sesspostdgst_tx
#define tx_table_s6_t2 s6_t2_nvme_sesspostdgst_tx
#define tx_table_s6_t3 s6_t3_nvme_sesspostdgst_tx

#define tx_table_s7_t0 s7_t0_nvme_sesspostdgst_tx
#define tx_table_s7_t1 s7_t1_nvme_sesspostdgst_tx
#define tx_table_s7_t2 s7_t2_nvme_sesspostdgst_tx
#define tx_table_s7_t3 s7_t3_nvme_sesspostdgst_tx

#define tx_stage0_lif_params_table lif_params_nvme_sesspostdgst_tx
#define tx_table_s5_t4_lif_rate_limiter_table lif_rate_limiter_nvme_sesspostdgst_tx

/**** action declarations ****/

#define tx_table_s0_t0_action cb_process

#define tx_table_s1_t0_action sess_wqe_process

//checks hdgst/ddgst status
#define tx_table_s2_t0_action pdu_ctxt1_process

//fetches num_pages and other cmd related parameters
#define tx_table_s3_t0_action pdu_ctxt0_process

//check out 'n' descriptors from tcp tx q
#define tx_table_s4_t0_action sessprodcb_process

//releases busy lock
#define tx_table_s5_t0_action cb_writeback_process

//prepares tcp tso descriptors
#define tx_table_s6_t0_action cb_tso_process_t0
#define tx_table_s6_t1_action cb_tso_process_t1

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

header_type nvme_sesspostdgst_tx_to_stage_sess_wqe_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_sesspostdgst_tx_to_stage_pdu_ctxt1_info_t {
    fields {
        pad                              :  128;
    }
}


header_type nvme_sesspostdgst_tx_to_stage_pdu_ctxt0_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_sesspostdgst_tx_to_stage_sessprodcb_info_t {
    fields {
        num_pages                        :  8;
        pad                              :  120;
    }
}

header_type nvme_sesspostdgst_tx_to_stage_writeback_info_t {
    fields {
        num_pages                        :  8;
        pduid                            :  16;
        pad                              :  104;
    }
}

header_type nvme_sesspostdgst_tx_to_stage_tso_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_sesspostdgst_tx_cb_to_sess_wqe_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_sesspostdgst_tx_sess_wqe_to_pdu_ctxt1_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_sesspostdgst_tx_pdu_ctxt1_to_pdu_ctxt0_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_sesspostdgst_tx_pdu_ctxt_to_sessprodcb_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_sesspostdgst_tx_sessprodcb_to_writeback_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_sesspostdgst_tx_writeback_to_tso_t {
    fields {
        num_pages           : 8;
        is_t0               : 1;
        tcp_wqe_offset      : 16;
        pad                 : 135;
    }
}

/**** scratch for D-vectors ****/

@pragma scratch_metadata
metadata sessdgsttxcb_t sessdgsttxcb_d;

@pragma scratch_metadata
metadata sess_wqe_t sess_wqe_d;

@pragma scratch_metadata
metadata pdu_context1_t pdu_ctxt1_d;

@pragma scratch_metadata
metadata pdu_context0_t pdu_ctxt0_d;

@pragma scratch_metadata
metadata sessprodcb_t sessprodcb_d;

@pragma scratch_metadata
metadata page_list_t page_list_d;

/**** global header unions ****/

@pragma pa_header_union ingress common_global
metadata phv_global_common_t phv_global_common;
@pragma scratch_metadata
metadata phv_global_common_t phv_global_common_scr;

/**** to stage header unions ****/

//To-Stage-0

//To-Stage-1
@pragma pa_header_union ingress to_stage_1
metadata nvme_sesspostdgst_tx_to_stage_sess_wqe_info_t to_s1_info;
@pragma scratch_metadata
metadata nvme_sesspostdgst_tx_to_stage_sess_wqe_info_t to_s1_info_scr;

//To-Stage-2
@pragma pa_header_union ingress to_stage_2
metadata nvme_sesspostdgst_tx_to_stage_pdu_ctxt1_info_t to_s2_info;
@pragma scratch_metadata
metadata nvme_sesspostdgst_tx_to_stage_pdu_ctxt1_info_t to_s2_info_scr;

//To-Stage-3
@pragma pa_header_union ingress to_stage_3
metadata nvme_sesspostdgst_tx_to_stage_pdu_ctxt0_info_t to_s3_info;
@pragma scratch_metadata
metadata nvme_sesspostdgst_tx_to_stage_pdu_ctxt0_info_t to_s3_info_scr;

//To-Stage-4
@pragma pa_header_union ingress to_stage_4
metadata nvme_sesspostdgst_tx_to_stage_sessprodcb_info_t to_s4_info;
@pragma scratch_metadata
metadata nvme_sesspostdgst_tx_to_stage_sessprodcb_info_t to_s4_info_scr;

//To-Stage-5
@pragma pa_header_union ingress to_stage_5
metadata nvme_sesspostdgst_tx_to_stage_writeback_info_t to_s5_info;
@pragma scratch_metadata
metadata nvme_sesspostdgst_tx_to_stage_writeback_info_t to_s5_info_scr;

//To-Stage-6
@pragma pa_header_union ingress to_stage_6
metadata nvme_sesspostdgst_tx_to_stage_tso_info_t to_s6_info;
@pragma scratch_metadata
metadata nvme_sesspostdgst_tx_to_stage_tso_info_t to_s6_info_scr;

/**** stage to stage header unions ****/

//Table-0
@pragma pa_header_union ingress common_t0_s2s t0_s2s_cb_to_sess_wqe_info t0_s2s_sess_wqe_to_pdu_ctxt1_info t0_s2s_pdu_ctxt1_to_pdu_ctxt0_info t0_s2s_pdu_ctxt_to_sessprodcb_info t0_s2s_writeback_to_tso_info t0_s2s_sessprodcb_to_writeback_info

metadata nvme_sesspostdgst_tx_cb_to_sess_wqe_t t0_s2s_cb_to_sess_wqe_info;
@pragma scratch_metadata
metadata nvme_sesspostdgst_tx_cb_to_sess_wqe_t t0_s2s_cb_to_sess_wqe_info_scr;

metadata nvme_sesspostdgst_tx_sess_wqe_to_pdu_ctxt1_t t0_s2s_sess_wqe_to_pdu_ctxt1_info;
@pragma scratch_metadata
metadata nvme_sesspostdgst_tx_sess_wqe_to_pdu_ctxt1_t t0_s2s_sess_wqe_to_pdu_ctxt1_info_scr;

metadata nvme_sesspostdgst_tx_pdu_ctxt1_to_pdu_ctxt0_t t0_s2s_pdu_ctxt1_to_pdu_ctxt0_info;
@pragma scratch_metadata
metadata nvme_sesspostdgst_tx_pdu_ctxt1_to_pdu_ctxt0_t t0_s2s_pdu_ctxt1_to_pdu_ctxt0_info_scr;

metadata nvme_sesspostdgst_tx_pdu_ctxt_to_sessprodcb_t t0_s2s_pdu_ctxt_to_sessprodcb_info;
@pragma scratch_metadata
metadata nvme_sesspostdgst_tx_pdu_ctxt_to_sessprodcb_t t0_s2s_pdu_ctxt_to_sessprodcb_info_scr;

metadata nvme_sesspostdgst_tx_writeback_to_tso_t t0_s2s_writeback_to_tso_info;
@pragma scratch_metadata
metadata nvme_sesspostdgst_tx_writeback_to_tso_t t0_s2s_writeback_to_tso_info_scr;

metadata nvme_sesspostdgst_tx_sessprodcb_to_writeback_t t0_s2s_sessprodcb_to_writeback_info;
@pragma scratch_metadata
metadata nvme_sesspostdgst_tx_sessprodcb_to_writeback_t t0_s2s_sessprodcb_to_writeback_info_scr;

//Table-1
@pragma pa_header_union ingress common_t1_s2s t1_s2s_writeback_to_tso_info

metadata nvme_sesspostdgst_tx_writeback_to_tso_t t1_s2s_writeback_to_tso_info;
@pragma scratch_metadata
metadata nvme_sesspostdgst_tx_writeback_to_tso_t t1_s2s_writeback_to_tso_info_scr;


//Table-2

/**** PHV Layout ****/
// PHV space for upto 16 TCP descriptors is reserved
// ASM code is expected to populate one or more of these descriptors, possibly
// using phvwrp 
// TODO: can we change it to any array format ?
@pragma pa_align 512
@pragma dont_trim
metadata hbm_al_ring_entry_t tcp_wqe0;
@pragma dont_trim
metadata hbm_al_ring_entry_t tcp_wqe1;
@pragma dont_trim
metadata hbm_al_ring_entry_t tcp_wqe2;
@pragma dont_trim
metadata hbm_al_ring_entry_t tcp_wqe3;
@pragma dont_trim
metadata hbm_al_ring_entry_t tcp_wqe4;
@pragma dont_trim
metadata hbm_al_ring_entry_t tcp_wqe5;
@pragma dont_trim
metadata hbm_al_ring_entry_t tcp_wqe6;
@pragma dont_trim
metadata hbm_al_ring_entry_t tcp_wqe7;
@pragma dont_trim
metadata hbm_al_ring_entry_t tcp_wqe8;
@pragma dont_trim
metadata hbm_al_ring_entry_t tcp_wqe9;
@pragma dont_trim
metadata hbm_al_ring_entry_t tcp_wqe10;
@pragma dont_trim
metadata hbm_al_ring_entry_t tcp_wqe11;
@pragma dont_trim
metadata hbm_al_ring_entry_t tcp_wqe12;
@pragma dont_trim
metadata hbm_al_ring_entry_t tcp_wqe13;
@pragma dont_trim
metadata hbm_al_ring_entry_t tcp_wqe14;
@pragma dont_trim
metadata hbm_al_ring_entry_t tcp_wqe15;
@pragma dont_trim
metadata data64_t tcp_db;
@pragma dont_trim
metadata index16_t sessdgst_q_cindex;

@pragma pa_align 128
//though 'n' tcp wqes are populated on PHV, assumption is that at the max 
//two DMA instructions are sufficient to download these to tcp transmit q
@pragma dont_trim
metadata dma_cmd_phv2mem_t tcp_wqe_dma0;            //dma cmd 0
@pragma dont_trim
metadata dma_cmd_phv2mem_t tcp_wqe_dma1;            //dma cmd 1
@pragma dont_trim
metadata dma_cmd_phv2mem_t tcp_db_dma;              //dma cmd 2
@pragma dont_trim
metadata dma_cmd_phv2mem_t sessdgst_q_cindex_dma;   //dma cmd 3

/*
 * Stage 0 table 0 action
 */
action cb_process (SESSDGSTTXCB_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage

    // D-vector
    GENERATE_SESSDGSTTXCB_D
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

action pdu_ctxt1_process (PDU_CTXT1_PARAMS) {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s2_info_scr.pad, to_s2_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_sess_wqe_to_pdu_ctxt1_info_scr.pad, t0_s2s_sess_wqe_to_pdu_ctxt1_info.pad);

    // D-vector
    GENERATE_PDU_CTXT1_D
}

action pdu_ctxt0_process (PDU_CTXT0_PARAMS) {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s3_info_scr.pad, to_s3_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_pdu_ctxt1_to_pdu_ctxt0_info_scr.pad, t0_s2s_pdu_ctxt1_to_pdu_ctxt0_info.pad);

    // D-vector
    GENERATE_PDU_CTXT0_D
}

action sessprodcb_process (SESSPRODCB_PARAMS) {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_info_scr.num_pages, to_s4_info.num_pages);
    modify_field(to_s4_info_scr.pad, to_s4_info.pad);

    // stage to stage
    modify_field(t0_s2s_pdu_ctxt_to_sessprodcb_info_scr.pad, t0_s2s_pdu_ctxt_to_sessprodcb_info.pad);

    // D-vector
    GENERATE_SESSPRODCB_D
}

action cb_writeback_process (SESSDGSTTXCB_PARAMS_NON_STG0) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s5_info_scr.num_pages, to_s5_info.num_pages);
    modify_field(to_s5_info_scr.pduid, to_s5_info.pduid);
    modify_field(to_s5_info_scr.pad, to_s5_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_sessprodcb_to_writeback_info_scr.pad, t0_s2s_sessprodcb_to_writeback_info.pad);

    GENERATE_SESSDGSTTXCB_D_NON_STG0
}

action cb_tso_process_t0 (PAGE_LIST_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s6_info_scr.pad, to_s6_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_writeback_to_tso_info_scr.num_pages, t0_s2s_writeback_to_tso_info.num_pages);
    modify_field(t0_s2s_writeback_to_tso_info_scr.is_t0, t0_s2s_writeback_to_tso_info.is_t0);
    modify_field(t0_s2s_writeback_to_tso_info_scr.tcp_wqe_offset, t0_s2s_writeback_to_tso_info.tcp_wqe_offset);
    modify_field(t0_s2s_writeback_to_tso_info_scr.pad, t0_s2s_writeback_to_tso_info.pad);

    GENERATE_PAGE_LIST_D
}

action cb_tso_process_t1 (PAGE_LIST_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s6_info_scr.pad, to_s6_info.pad);

    // stage to stage
    modify_field(t1_s2s_writeback_to_tso_info_scr.num_pages, t1_s2s_writeback_to_tso_info.num_pages);
    modify_field(t1_s2s_writeback_to_tso_info_scr.is_t0, t1_s2s_writeback_to_tso_info.is_t0);
    modify_field(t1_s2s_writeback_to_tso_info_scr.tcp_wqe_offset, t1_s2s_writeback_to_tso_info.tcp_wqe_offset);
    modify_field(t1_s2s_writeback_to_tso_info_scr.pad, t1_s2s_writeback_to_tso_info.pad);

    GENERATE_PAGE_LIST_D
}
