/***********************************************************************/
/* nvme_req_txdma.p4 */
/***********************************************************************/

#include "../txdma/common_txdma_dummy.p4"

/**** table declarations ****/

#define tx_table_s0_t0 nvme_sessxts_tx_s0_t0
#define tx_table_s0_t1 nvme_sessxts_tx_s0_t1
#define tx_table_s0_t2 nvme_sessxts_tx_s0_t2

#define tx_table_s1_t0 nvme_sessxts_tx_s1_t0
#define tx_table_s1_t1 nvme_sessxts_tx_s1_t1
#define tx_table_s1_t2 nvme_sessxts_tx_s1_t2

#define tx_table_s2_t0 nvme_sessxts_tx_s2_t0
#define tx_table_s2_t1 nvme_sessxts_tx_s2_t1
#define tx_table_s2_t2 nvme_sessxts_tx_s2_t2

#define tx_table_s3_t0 nvme_sessxts_tx_s3_t0
#define tx_table_s3_t1 nvme_sessxts_tx_s3_t1
#define tx_table_s3_t2 nvme_sessxts_tx_s3_t2

#define tx_table_s4_t0 nvme_sessxts_tx_s4_t0
#define tx_table_s4_t1 nvme_sessxts_tx_s4_t1
#define tx_table_s4_t2 nvme_sessxts_tx_s4_t2

#define tx_table_s5_t0 nvme_sessxts_tx_s5_t0
#define tx_table_s5_t1 nvme_sessxts_tx_s5_t1
#define tx_table_s5_t2 nvme_sessxts_tx_s5_t2

#define tx_table_s6_t0 nvme_sessxts_tx_s6_t0
#define tx_table_s6_t1 nvme_sessxts_tx_s6_t1
#define tx_table_s6_t2 nvme_sessxts_tx_s6_t2

#define tx_table_s7_t0 nvme_sessxts_tx_s7_t0
#define tx_table_s7_t1 nvme_sessxts_tx_s7_t1
#define tx_table_s7_t2 nvme_sessxts_tx_s7_t2

/**** action declarations ****/

#define tx_table_s0_t0_action nvme_sessxts_tx_cb_process

#define tx_table_s1_t0_action nvme_sessxts_tx_sess_wqe_process
#define tx_table_s1_t1_action nvme_sessxts_tx_xtscb_process

#define tx_table_s2_t0_action nvme_sessxts_tx_nvme_wqe_process

#define tx_table_s3_t0_action nvme_sessxts_tx_cb_writeback_process

#define tx_table_s4_t0_action nvme_sessxts_tx_ip_desc_process
#define tx_table_s4_t1_action nvme_sessxts_tx_op_desc_process

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

header_type nvme_sessxts_tx_to_stage_sess_wqe_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_sessxts_tx_to_stage_nvme_wqe_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_sessxts_tx_to_stage_writeback_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_sessxts_tx_to_stage_ip_op_desc_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_sessxts_tx_cb_to_sess_wqe_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_sessxts_tx_cb_to_xtscb_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_sessxts_tx_sess_wqe_to_nvme_wqe_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_sessxts_tx_nvme_wqe_to_writeback_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_sessxts_tx_writeback_to_ip_desc_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_sessxts_tx_writeback_to_op_desc_t {
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
metadata xtscb_t xtscb_d;

@pragma scratch_metadata
metadata nvme_wqe_t nvme_wqe_d;

/**** global header unions ****/

@pragma pa_header_union ingress common_global
metadata phv_global_common_t phv_global_common;
@pragma scratch_metadata
metadata phv_global_common_t phv_global_common_scr;

/**** to stage header unions ****/

//To-Stage-0

//To-Stage-1
@pragma pa_header_union ingress to_stage_1
metadata nvme_sessxts_tx_to_stage_sess_wqe_info_t to_s1_info;
@pragma scratch_metadata
metadata nvme_sessxts_tx_to_stage_sess_wqe_info_t to_s1_info_scr;

//To-Stage-2
@pragma pa_header_union ingress to_stage_2
metadata nvme_sessxts_tx_to_stage_nvme_wqe_info_t to_s2_info;
@pragma scratch_metadata
metadata nvme_sessxts_tx_to_stage_nvme_wqe_info_t to_s2_info_scr;

//To-Stage-3
@pragma pa_header_union ingress to_stage_3
metadata nvme_sessxts_tx_to_stage_writeback_info_t to_s3_info;
@pragma scratch_metadata
metadata nvme_sessxts_tx_to_stage_writeback_info_t to_s3_info_scr;

//To-Stage-4
@pragma pa_header_union ingress to_stage_4
metadata nvme_sessxts_tx_to_stage_ip_op_desc_info_t to_s4_info;
@pragma scratch_metadata
metadata nvme_sessxts_tx_to_stage_ip_op_desc_info_t to_s4_info_scr;

/**** stage to stage header unions ****/

//Table-0
@pragma pa_header_union ingress common_t0_s2s t0_s2s_cb_to_sess_wqe_info t0_s2s_sess_wqe_to_nvme_wqe_info t0_s2s_nvme_wqe_to_writeback_info t0_s2s_writeback_to_ip_desc_info

metadata nvme_sessxts_tx_cb_to_sess_wqe_t t0_s2s_cb_to_sess_wqe_info;
@pragma scratch_metadata
metadata nvme_sessxts_tx_cb_to_sess_wqe_t t0_s2s_cb_to_sess_wqe_info_scr;

metadata nvme_sessxts_tx_sess_wqe_to_nvme_wqe_t t0_s2s_sess_wqe_to_nvme_wqe_info;
@pragma scratch_metadata
metadata nvme_sessxts_tx_sess_wqe_to_nvme_wqe_t t0_s2s_sess_wqe_to_nvme_wqe_info_scr;

metadata nvme_sessxts_tx_nvme_wqe_to_writeback_t t0_s2s_nvme_wqe_to_writeback_info;
@pragma scratch_metadata
metadata nvme_sessxts_tx_nvme_wqe_to_writeback_t t0_s2s_nvme_wqe_to_writeback_info_scr;

metadata nvme_sessxts_tx_writeback_to_ip_desc_t t0_s2s_writeback_to_ip_desc_info;
@pragma scratch_metadata
metadata nvme_sessxts_tx_writeback_to_ip_desc_t t0_s2s_writeback_to_ip_desc_info_scr;

//Table-1
@pragma pa_header_union ingress common_t1_s2s t1_s2s_cb_to_xtscb_info t1_s2s_writeback_to_op_desc_info 

metadata nvme_sessxts_tx_cb_to_xtscb_t t1_s2s_cb_to_xtscb_info;
@pragma scratch_metadata
metadata nvme_sessxts_tx_cb_to_xtscb_t t1_s2s_cb_to_xtscb_info_scr;

metadata nvme_sessxts_tx_writeback_to_op_desc_t t1_s2s_writeback_to_op_desc_info;
@pragma scratch_metadata
metadata nvme_sessxts_tx_writeback_to_op_desc_t t1_s2s_writeback_to_op_desc_info_scr;


//Table-2

/**** PHV Layout ****/
@pragma dont_trim
metadata barco_desc_t barco_desc;
@pragma dont_trim
metadata pkt_descr_aol_t ip_desc;
@pragma dont_trim
metadata pkt_descr_aol_t op_desc;
@pragma dont_trim
metadata barco_iv_t iv;
@pragma dont_trim
metadata doorbell_data_t barco_db;
@pragma dont_trim
metadata ptr64_t dgst_ptr0;
@pragma dont_trim
metadata ptr64_t dgst_ptr1;

@pragma pa_align 128
@pragma dont_trim
metadata dma_cmd_phv2mem_t ip_desc_dma;         //dma cmd 0
@pragma dont_trim
metadata dma_cmd_phv2mem_t op_desc_dma;         //dma cmd 1
@pragma dont_trim
metadata dma_cmd_phv2mem_t iv_dma;              //dma cmd 2
@pragma dont_trim
metadata dma_cmd_phv2mem_t barco_desc_dma;      //dma cmd 3
@pragma dont_trim
metadata dma_cmd_phv2mem_t barco_db_dma;        //dma cmd 4
@pragma dont_trim
metadata dma_cmd_phv2mem_t dgst_ptr0_dma;       //dma cmd 5
@pragma dont_trim
metadata dma_cmd_phv2mem_t dgst_ptr1_dma;       //dma cmd 6

/*
 * Stage 0 table 0 action
 */
action nvme_sessxts_tx_cb_process (SESSXTSTXCB_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage

    // D-vector
    GENERATE_SESSXTSTXCB_D
}

action nvme_sessxts_tx_sess_wqe_process (SESS_WQE_PARAMS) {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_info_scr.pad, to_s1_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_cb_to_sess_wqe_info_scr.pad, t0_s2s_cb_to_sess_wqe_info.pad);

    // D-vector
    GENERATE_SESS_WQE_D
}

action nvme_sessxts_tx_xtscb_process (XTSCB_PARAMS) {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_info_scr.pad, to_s1_info.pad);
    
    // stage to stage
    modify_field(t1_s2s_cb_to_xtscb_info_scr.pad, t1_s2s_cb_to_xtscb_info.pad);

    // D-vector
    GENERATE_XTSCB_D
}

action nvme_sessxts_tx_nvme_wqe_process (NVME_WQE_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s2_info_scr.pad, to_s2_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_sess_wqe_to_nvme_wqe_info_scr.pad, t0_s2s_sess_wqe_to_nvme_wqe_info.pad);

    // D-vector
    GENERATE_NVME_WQE_D
}

action nvme_sessxts_tx_cb_writeback_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s3_info_scr.pad, to_s3_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_nvme_wqe_to_writeback_info_scr.pad, t0_s2s_nvme_wqe_to_writeback_info.pad);
}


action nvme_sessxts_tx_ip_desc_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_info_scr.pad, to_s4_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_writeback_to_ip_desc_info_scr.pad, t0_s2s_writeback_to_ip_desc_info.pad);
}

action nvme_sessxts_tx_op_desc_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_info_scr.pad, to_s4_info.pad);
    
    // stage to stage
    modify_field(t1_s2s_writeback_to_op_desc_info_scr.pad, t1_s2s_writeback_to_op_desc_info.pad);
}
