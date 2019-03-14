/***********************************************************************/
/* nvme_sess_pre_dgst_tx.p4 */
/***********************************************************************/

#include "../txdma/common_txdma_dummy.p4"

/**** table declarations ****/

#define tx_table_s0_t0 nvme_sesspredgst_tx_s0_t0
#define tx_table_s0_t1 nvme_sesspredgst_tx_s0_t1
#define tx_table_s0_t2 nvme_sesspredgst_tx_s0_t2

#define tx_table_s1_t0 nvme_sesspredgst_tx_s1_t0
#define tx_table_s1_t1 nvme_sesspredgst_tx_s1_t1
#define tx_table_s1_t2 nvme_sesspredgst_tx_s1_t2

#define tx_table_s2_t0 nvme_sesspredgst_tx_s2_t0
#define tx_table_s2_t1 nvme_sesspredgst_tx_s2_t1
#define tx_table_s2_t2 nvme_sesspredgst_tx_s2_t2

#define tx_table_s3_t0 nvme_sesspredgst_tx_s3_t0
#define tx_table_s3_t1 nvme_sesspredgst_tx_s3_t1
#define tx_table_s3_t2 nvme_sesspredgst_tx_s3_t2

#define tx_table_s4_t0 nvme_sesspredgst_tx_s4_t0
#define tx_table_s4_t1 nvme_sesspredgst_tx_s4_t1
#define tx_table_s4_t2 nvme_sesspredgst_tx_s4_t2

#define tx_table_s5_t0 nvme_sesspredgst_tx_s5_t0
#define tx_table_s5_t1 nvme_sesspredgst_tx_s5_t1
#define tx_table_s5_t2 nvme_sesspredgst_tx_s5_t2

#define tx_table_s6_t0 nvme_sesspredgst_tx_s6_t0
#define tx_table_s6_t1 nvme_sesspredgst_tx_s6_t1
#define tx_table_s6_t2 nvme_sesspredgst_tx_s6_t2

#define tx_table_s7_t0 nvme_sesspredgst_tx_s7_t0
#define tx_table_s7_t1 nvme_sesspredgst_tx_s7_t1
#define tx_table_s7_t2 nvme_sesspredgst_tx_s7_t2

/**** action declarations ****/

#define tx_table_s0_t0_action nvme_sesspredgst_tx_cb_process

#define tx_table_s1_t0_action nvme_sesspredgst_tx_sess_wqe_process

#define tx_table_s2_t0_action nvme_sesspredgst_tx_nvme_wqe_process

#define tx_table_s3_t0_action nvme_sesspredgst_tx_dgstcb_process

#define tx_table_s4_t0_action nvme_sesspredgst_tx_cb_writeback_process

#define tx_table_s5_t0_action nvme_sesspredgst_tx_hdgst_process
#define tx_table_s5_t1_action nvme_sesspredgst_tx_ddgst_process

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

header_type nvme_sesspredgst_tx_to_stage_sess_wqe_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_sesspredgst_tx_to_stage_nvme_wqe_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_sesspredgst_tx_to_stage_dgstcb_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_sesspredgst_tx_to_stage_writeback_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_sesspredgst_tx_to_stage_hdgst_ddgst_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_sesspredgst_tx_cb_to_sess_wqe_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_sesspredgst_tx_sess_wqe_to_nvme_wqe_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_sesspredgst_tx_nvme_wqe_to_dgstcb_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_sesspredgst_tx_dgstcb_to_writeback_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_sesspredgst_tx_writeback_to_hdgst_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_sesspredgst_tx_writeback_to_ddgst_t {
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
metadata dgstcb_t dgstcb_d;

/**** global header unions ****/

@pragma pa_header_union ingress common_global
metadata phv_global_common_t phv_global_common;
@pragma scratch_metadata
metadata phv_global_common_t phv_global_common_scr;

/**** to stage header unions ****/

//To-Stage-0

//To-Stage-1
@pragma pa_header_union ingress to_stage_1
metadata nvme_sesspredgst_tx_to_stage_sess_wqe_info_t to_s1_info;
@pragma scratch_metadata
metadata nvme_sesspredgst_tx_to_stage_sess_wqe_info_t to_s1_info_scr;

//To-Stage-2
@pragma pa_header_union ingress to_stage_2
metadata nvme_sesspredgst_tx_to_stage_nvme_wqe_info_t to_s2_info;
@pragma scratch_metadata
metadata nvme_sesspredgst_tx_to_stage_nvme_wqe_info_t to_s2_info_scr;

//To-Stage-3
@pragma pa_header_union ingress to_stage_3
metadata nvme_sesspredgst_tx_to_stage_dgstcb_info_t to_s3_info;
@pragma scratch_metadata
metadata nvme_sesspredgst_tx_to_stage_dgstcb_info_t to_s3_info_scr;

//To-Stage-4
@pragma pa_header_union ingress to_stage_4
metadata nvme_sesspredgst_tx_to_stage_writeback_info_t to_s4_info;
@pragma scratch_metadata
metadata nvme_sesspredgst_tx_to_stage_writeback_info_t to_s4_info_scr;

//To-Stage-5
@pragma pa_header_union ingress to_stage_5
metadata nvme_sesspredgst_tx_to_stage_hdgst_ddgst_info_t to_s5_info;
@pragma scratch_metadata
metadata nvme_sesspredgst_tx_to_stage_hdgst_ddgst_info_t to_s5_info_scr;

/**** stage to stage header unions ****/

//Table-0
@pragma pa_header_union ingress common_t0_s2s t0_s2s_cb_to_sess_wqe_info t0_s2s_sess_wqe_to_nvme_wqe_info t0_s2s_nvme_wqe_to_dgstcb_info t0_s2s_dgstcb_to_writeback_info t0_s2s_writeback_to_hdgst_info

metadata nvme_sesspredgst_tx_cb_to_sess_wqe_t t0_s2s_cb_to_sess_wqe_info;
@pragma scratch_metadata
metadata nvme_sesspredgst_tx_cb_to_sess_wqe_t t0_s2s_cb_to_sess_wqe_info_scr;

metadata nvme_sesspredgst_tx_sess_wqe_to_nvme_wqe_t t0_s2s_sess_wqe_to_nvme_wqe_info;
@pragma scratch_metadata
metadata nvme_sesspredgst_tx_sess_wqe_to_nvme_wqe_t t0_s2s_sess_wqe_to_nvme_wqe_info_scr;

metadata nvme_sesspredgst_tx_nvme_wqe_to_dgstcb_t t0_s2s_nvme_wqe_to_dgstcb_info;
@pragma scratch_metadata
metadata nvme_sesspredgst_tx_nvme_wqe_to_dgstcb_t t0_s2s_nvme_wqe_to_dgstcb_info_scr;

metadata nvme_sesspredgst_tx_dgstcb_to_writeback_t t0_s2s_dgstcb_to_writeback_info;
@pragma scratch_metadata
metadata nvme_sesspredgst_tx_dgstcb_to_writeback_t t0_s2s_dgstcb_to_writeback_info_scr;

metadata nvme_sesspredgst_tx_writeback_to_hdgst_t t0_s2s_writeback_to_hdgst_info;
@pragma scratch_metadata
metadata nvme_sesspredgst_tx_writeback_to_hdgst_t t0_s2s_writeback_to_hdgst_info_scr;

//Table-1
@pragma pa_header_union ingress common_t1_s2s t1_s2s_writeback_to_ddgst_info 

metadata nvme_sesspredgst_tx_writeback_to_ddgst_t t1_s2s_writeback_to_ddgst_info;
@pragma scratch_metadata
metadata nvme_sesspredgst_tx_writeback_to_ddgst_t t1_s2s_writeback_to_ddgst_info_scr;


//Table-2

/**** PHV Layout ****/
@pragma dont_trim
metadata dgst_desc_t ddgst_desc;
@pragma dont_trim
metadata dgst_aol_desc_t ddgst_aol_desc;    //Do we need it ?
@pragma dont_trim
metadata dgst_desc_t hdgst_desc;
@pragma dont_trim
metadata dgst_aol_desc_t hdgst_aol_desc;
@pragma dont_trim
metadata doorbell_data_t dgst_db;

@pragma pa_align 128
@pragma dont_trim
metadata dma_cmd_phv2mem_t ddgst_aol_desc_dma;  //dma cmd 0
@pragma dont_trim
metadata dma_cmd_phv2mem_t ddgst_desc_dma;      //dma cmd 1
@pragma dont_trim
metadata dma_cmd_phv2mem_t hdgst_aol_desc_dma;  //dma cmd 2
@pragma dont_trim
metadata dma_cmd_phv2mem_t hdgst_desc_dma;      //dma cmd 3
@pragma dont_trim
metadata dma_cmd_phv2mem_t dgst_db_dma;         //dma cmd 4

/*
 * Stage 0 table 0 action
 */
action nvme_sesspredgst_tx_cb_process (SESSDGSTTXCB_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage

    // D-vector
    GENERATE_SESSDGSTTXCB_D
}

action nvme_sesspredgst_tx_sess_wqe_process (SESS_WQE_PARAMS) {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_info_scr.pad, to_s1_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_cb_to_sess_wqe_info_scr.pad, t0_s2s_cb_to_sess_wqe_info.pad);

    // D-vector
    GENERATE_SESS_WQE_D
}

action nvme_sesspredgst_tx_nvme_wqe_process (NVME_WQE_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s2_info_scr.pad, to_s2_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_sess_wqe_to_nvme_wqe_info_scr.pad, t0_s2s_sess_wqe_to_nvme_wqe_info.pad);

    // D-vector
    GENERATE_NVME_WQE_D
}

action nvme_sesspredgst_tx_dgstcb_process (DGSTCB_PARAMS) {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s3_info_scr.pad, to_s3_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_nvme_wqe_to_dgstcb_info_scr.pad, t0_s2s_nvme_wqe_to_dgstcb_info.pad);

    // D-vector
    GENERATE_DGSTCB_D
}

action nvme_sesspredgst_tx_cb_writeback_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_info_scr.pad, to_s4_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_dgstcb_to_writeback_info_scr.pad, t0_s2s_dgstcb_to_writeback_info.pad);
}


action nvme_sesspredgst_tx_hdgst_process() {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s5_info_scr.pad, to_s5_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_writeback_to_hdgst_info_scr.pad, t0_s2s_writeback_to_hdgst_info.pad);
}

action nvme_sesspredgst_tx_ddgst_process() {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s5_info_scr.pad, to_s5_info.pad);
    
    // stage to stage
    modify_field(t1_s2s_writeback_to_ddgst_info_scr.pad, t1_s2s_writeback_to_ddgst_info.pad);
}
