/***********************************************************************/
/* nvme_sess_post_xts_tx.p4 */
/***********************************************************************/

#include "../txdma/common_txdma_dummy.p4"

/**** table declarations ****/

#define tx_table_s0_t0 nvme_sesspostxts_tx_s0_t0
#define tx_table_s0_t1 nvme_sesspostxts_tx_s0_t1
#define tx_table_s0_t2 nvme_sesspostxts_tx_s0_t2

#define tx_table_s1_t0 nvme_sesspostxts_tx_s1_t0
#define tx_table_s1_t1 nvme_sesspostxts_tx_s1_t1
#define tx_table_s1_t2 nvme_sesspostxts_tx_s1_t2

#define tx_table_s2_t0 nvme_sesspostxts_tx_s2_t0
#define tx_table_s2_t1 nvme_sesspostxts_tx_s2_t1
#define tx_table_s2_t2 nvme_sesspostxts_tx_s2_t2

#define tx_table_s3_t0 nvme_sesspostxts_tx_s3_t0
#define tx_table_s3_t1 nvme_sesspostxts_tx_s3_t1
#define tx_table_s3_t2 nvme_sesspostxts_tx_s3_t2

#define tx_table_s4_t0 nvme_sesspostxts_tx_s4_t0
#define tx_table_s4_t1 nvme_sesspostxts_tx_s4_t1
#define tx_table_s4_t2 nvme_sesspostxts_tx_s4_t2

#define tx_table_s5_t0 nvme_sesspostxts_tx_s5_t0
#define tx_table_s5_t1 nvme_sesspostxts_tx_s5_t1
#define tx_table_s5_t2 nvme_sesspostxts_tx_s5_t2

#define tx_table_s6_t0 nvme_sesspostxts_tx_s6_t0
#define tx_table_s6_t1 nvme_sesspostxts_tx_s6_t1
#define tx_table_s6_t2 nvme_sesspostxts_tx_s6_t2

#define tx_table_s7_t0 nvme_sesspostxts_tx_s7_t0
#define tx_table_s7_t1 nvme_sesspostxts_tx_s7_t1
#define tx_table_s7_t2 nvme_sesspostxts_tx_s7_t2

/**** action declarations ****/

#define tx_table_s0_t0_action nvme_sesspostxts_tx_cb_process

#define tx_table_s1_t0_action nvme_sesspostxts_tx_sess_wqe_process

//this stage/table should be in-sync with nvme_req_tx_sessprodcb_process
//for locked table purpose as both the programs produce into dgst q
#define tx_table_s5_t0_action nvme_sesspostxts_tx_sessprodcb_process

#define tx_table_s6_t0_action nvme_sesspostxts_tx_cb_writeback_process

#include "../txdma/common_txdma.p4"
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
        pad                              :  128;
    }
}

header_type nvme_sesspostxts_tx_to_stage_sessprodcb_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_sesspostxts_tx_to_stage_writeback_info_t {
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

/**** scratch for D-vectors ****/

@pragma scratch_metadata
metadata sessxtstxcb_t sessxtstxcb_d;

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

/**** stage to stage header unions ****/

//Table-0
@pragma pa_header_union ingress common_t0_s2s t0_s2s_cb_to_sess_wqe_info t0_s2s_sess_wqe_to_sessprodcb_info t0_s2s_sessprodcb_to_writeback_info

metadata nvme_sesspostxts_tx_cb_to_sess_wqe_t t0_s2s_cb_to_sess_wqe_info;
@pragma scratch_metadata
metadata nvme_sesspostxts_tx_cb_to_sess_wqe_t t0_s2s_cb_to_sess_wqe_info_scr;

metadata nvme_sesspostxts_tx_sess_wqe_to_sessprodcb_t t0_s2s_sess_wqe_to_sessprodcb_info;
@pragma scratch_metadata
metadata nvme_sesspostxts_tx_sess_wqe_to_sessprodcb_t t0_s2s_sess_wqe_to_sessprodcb_info_scr;

metadata nvme_sesspostxts_tx_sessprodcb_to_writeback_t t0_s2s_sessprodcb_to_writeback_info;
@pragma scratch_metadata
metadata nvme_sesspostxts_tx_sessprodcb_to_writeback_t t0_s2s_sessprodcb_to_writeback_info_scr;

//Table-1

//Table-2

/**** PHV Layout ****/
@pragma dont_trim
metadata sess_wqe_t sess_wqe;
@pragma dont_trim
metadata doorbell_data_t session_db;
@pragma dont_trim
metadata index16_t sessxts_q_cindex;

@pragma pa_align 128
@pragma dont_trim
metadata dma_cmd_phv2mem_t session_wqe_dma;         //dma cmd 0
@pragma dont_trim
metadata dma_cmd_phv2mem_t session_db_dma;          //dma cmd 1
@pragma dont_trim
metadata dma_cmd_phv2mem_t sessxts_q_cindex_dma;    //dma cmd 2

/*
 * Stage 0 table 0 action
 */
action nvme_sesspostxts_tx_cb_process (SESSXTSTXCB_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage

    // D-vector
    GENERATE_SESSXTSTXCB_D
}

action nvme_sesspostxts_tx_sess_wqe_process (SESS_WQE_PARAMS) {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_info_scr.pad, to_s1_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_cb_to_sess_wqe_info_scr.pad, t0_s2s_cb_to_sess_wqe_info.pad);

    // D-vector
    GENERATE_SESS_WQE_D
}

action nvme_sesspostxts_tx_sessprodcb_process (SESSPRODCB_PARAMS) {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s5_info_scr.pad, to_s5_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_sess_wqe_to_sessprodcb_info_scr.pad, t0_s2s_sess_wqe_to_sessprodcb_info.pad);

    // D-vector
    GENERATE_SESSPRODCB_D
}

action nvme_sesspostxts_tx_cb_writeback_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s6_info_scr.pad, to_s6_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_sessprodcb_to_writeback_info_scr.pad, t0_s2s_sessprodcb_to_writeback_info.pad);
}
