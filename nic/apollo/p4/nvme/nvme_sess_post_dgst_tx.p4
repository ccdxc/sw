/***********************************************************************/
/* nvme_sess_post_xts_tx.p4 */
/***********************************************************************/

#include "common_txdma_dummy.p4"

/**** table declarations ****/

#define tx_table_s0_t0 nvme_sesspostdgst_tx_s0_t0
#define tx_table_s0_t1 nvme_sesspostdgst_tx_s0_t1
#define tx_table_s0_t2 nvme_sesspostdgst_tx_s0_t2

#define tx_table_s1_t0 nvme_sesspostdgst_tx_s1_t0
#define tx_table_s1_t1 nvme_sesspostdgst_tx_s1_t1
#define tx_table_s1_t2 nvme_sesspostdgst_tx_s1_t2

#define tx_table_s2_t0 nvme_sesspostdgst_tx_s2_t0
#define tx_table_s2_t1 nvme_sesspostdgst_tx_s2_t1
#define tx_table_s2_t2 nvme_sesspostdgst_tx_s2_t2

#define tx_table_s3_t0 nvme_sesspostdgst_tx_s3_t0
#define tx_table_s3_t1 nvme_sesspostdgst_tx_s3_t1
#define tx_table_s3_t2 nvme_sesspostdgst_tx_s3_t2

#define tx_table_s4_t0 nvme_sesspostdgst_tx_s4_t0
#define tx_table_s4_t1 nvme_sesspostdgst_tx_s4_t1
#define tx_table_s4_t2 nvme_sesspostdgst_tx_s4_t2

#define tx_table_s5_t0 nvme_sesspostdgst_tx_s5_t0
#define tx_table_s5_t1 nvme_sesspostdgst_tx_s5_t1
#define tx_table_s5_t2 nvme_sesspostdgst_tx_s5_t2

#define tx_table_s6_t0 nvme_sesspostdgst_tx_s6_t0
#define tx_table_s6_t1 nvme_sesspostdgst_tx_s6_t1
#define tx_table_s6_t2 nvme_sesspostdgst_tx_s6_t2

#define tx_table_s7_t0 nvme_sesspostdgst_tx_s7_t0
#define tx_table_s7_t1 nvme_sesspostdgst_tx_s7_t1
#define tx_table_s7_t2 nvme_sesspostdgst_tx_s7_t2

/**** action declarations ****/

#define tx_table_s0_t0_action nvme_sesspostdgst_tx_cb_process

#define tx_table_s1_t0_action nvme_sesspostdgst_tx_sess_wqe_process

//checks hdgst/ddgst status
#define tx_table_s2_t0_action nvme_sesspostdgst_tx_nvme_wqe_process

//check out 'n' descriptors from tcp tx q
#define tx_table_s3_t0_action nvme_sesspostdgst_tx_sessprodcb_process

//releases busy lock
#define tx_table s4_t0_action nvme_sesspostdgst_tx_cb_writeback_process

//prepares tcp tso descriptors
#define tx_table_s5_t0_action nvme_sesspostdgst_tx_cb_tso_process

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

header_type nvme_sesspostdgst_tx_to_stage_nvme_wqe_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_sesspostdgst_tx_to_stage_sessprodcb_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_sesspostdgst_tx_to_stage_writeback_info_t {
    fields {
        pad                              :  128;
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

header_type nvme_sesspostdgst_tx_sess_wqe_to_nvme_wqe_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_sesspostdgst_tx_nvme_wqe_to_sessprodcb_t {
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
        pad                 : 160;
    }
}

/**** scratch for D-vectors ****/

@pragma scratch_metadata
metadata sessdgsttxcb_t sessdgsttxcb_d;

@pragma scratch_metadata
metadata sess_wqe_t sess_wqe_d;

@pragma scratch_metadata
metadata nvme_wqe_t nvme_wqe_d;

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
metadata nvme_sesspostdgst_tx_to_stage_sess_wqe_info_t to_s1_info;
@pragma scratch_metadata
metadata nvme_sesspostdgst_tx_to_stage_sess_wqe_info_t to_s1_info_scr;

//To-Stage-2
@pragma pa_header_union ingress to_stage_2
metadata nvme_sesspostdgst_tx_to_stage_nvme_wqe_info_t to_s2_info;
@pragma scratch_metadata
metadata nvme_sesspostdgst_tx_to_stage_nvme_wqe_info_t to_s2_info_scr;

//To-Stage-3
@pragma pa_header_union ingress to_stage_3
metadata nvme_sesspostdgst_tx_to_stage_sessprodcb_info_t to_s3_info;
@pragma scratch_metadata
metadata nvme_sesspostdgst_tx_to_stage_sessprodcb_info_t to_s3_info_scr;

//To-Stage-4
@pragma pa_header_union ingress to_stage_4
metadata nvme_sesspostdgst_tx_to_stage_writeback_info_t to_s4_info;
@pragma scratch_metadata
metadata nvme_sesspostdgst_tx_to_stage_writeback_info_t to_s4_info_scr;

//To-Stage-5
@pragma pa_header_union ingress to_stage_5
metadata nvme_sesspostdgst_tx_to_stage_tso_info_t to_s5_info;
@pragma scratch_metadata
metadata nvme_sesspostdgst_tx_to_stage_tso_info_t to_s5_info_scr;

/**** stage to stage header unions ****/

//Table-0
@pragma pa_header_union ingress common_t0_s2s t0_s2s_cb_to_sess_wqe_info t0_s2s_sess_wqe_to_nvme_wqe_info t0_s2s_nvme_wqe_to_sessprodcb_info t0_s2s_writeback_to_tso_info t0_s2s_sessprodcb_to_writeback_info

metadata nvme_sesspostdgst_tx_cb_to_sess_wqe_t t0_s2s_cb_to_sess_wqe_info;
@pragma scratch_metadata
metadata nvme_sesspostdgst_tx_cb_to_sess_wqe_t t0_s2s_cb_to_sess_wqe_info_scr;

metadata nvme_sesspostdgst_tx_sess_wqe_to_nvme_wqe_t t0_s2s_sess_wqe_to_nvme_wqe_info;
@pragma scratch_metadata
metadata nvme_sesspostdgst_tx_sess_wqe_to_nvme_wqe_t t0_s2s_sess_wqe_to_nvme_wqe_info_scr;

metadata nvme_sesspostdgst_tx_nvme_wqe_to_sessprodcb_t t0_s2s_nvme_wqe_to_sessprodcb_info;
@pragma scratch_metadata
metadata nvme_sesspostdgst_tx_nvme_wqe_to_sessprodcb_t t0_s2s_nvme_wqe_to_sessprodcb_info_scr;

metadata nvme_sesspostdgst_tx_writeback_to_tso_t t0_s2s_writeback_to_tso_info;
@pragma scratch_metadata
metadata nvme_sesspostdgst_tx_writeback_to_tso_t t0_s2s_writeback_to_tso_info_scr;

@pragma dont_trim
metadata nvme_sesspostdgst_tx_sessprodcb_to_writeback_t t0_s2s_sessprodcb_to_writeback_info;
@pragma scratch_metadata
metadata nvme_sesspostdgst_tx_sessprodcb_to_writeback_t t0_s2s_sessprodcb_to_writeback_info_scr;

//Table-1

//Table-2

/**** PHV Layout ****/
// PHV space for upto 16 TCP descriptors is reserved
// ASM code is expected to populate one or more of these descriptors, possibly
// using phvwrp 
// TODO: can we change it to any array format ?
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
metadata doorbell_data_t tcp_db;
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
action nvme_sesspostdgst_tx_cb_process (SESSDGSTTXCB_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage

    // D-vector
    GENERATE_SESSDGSTTXCB_D
}

action nvme_sesspostdgst_tx_sess_wqe_process (SESS_WQE_PARAMS) {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_info_scr.pad, to_s1_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_cb_to_sess_wqe_info_scr.pad, t0_s2s_cb_to_sess_wqe_info.pad);

    // D-vector
    GENERATE_SESS_WQE_D
}

action nvme_sesspostdgst_tx_nvme_wqe_process (NVME_WQE_PARAMS) {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s2_info_scr.pad, to_s2_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_sess_wqe_to_nvme_wqe_info_scr.pad, t0_s2s_sess_wqe_to_nvme_wqe_info.pad);

    // D-vector
    GENERATE_NVME_WQE_D
}

action nvme_sesspostdgst_tx_sessprodcb_process (SESSPRODCB_PARAMS) {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s3_info_scr.pad, to_s3_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_nvme_wqe_to_sessprodcb_info_scr.pad, t0_s2s_nvme_wqe_to_sessprodcb_info.pad);

    // D-vector
    GENERATE_SESSPRODCB_D
}

action nvme_sesspostdgst_tx_cb_writeback_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_info_scr.pad, to_s4_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_sessprodcb_to_writeback_info_scr.pad, t0_s2s_sessprodcb_to_writeback_info.pad);
}

action nvme_sesspostdgst_tx_cb_tso_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s5_info_scr.pad, to_s5_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_writeback_to_tso_info_scr.pad, t0_s2s_writeback_to_tso_info.pad);
}
