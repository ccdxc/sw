/***********************************************************************/
/* nvme_req_tx.p4 */
/***********************************************************************/

#include "common_txdma_dummy.p4"

/**** table declarations ****/

#define tx_table_s0_t0 s0_t0
#define tx_table_s0_t1 s0_t1
#define tx_table_s0_t2 s0_t2

#define tx_table_s1_t0 s1_t0
#define tx_table_s1_t1 s1_t1
#define tx_table_s1_t2 s1_t2

#define tx_table_s2_t0 s2_t0
#define tx_table_s2_t1 s2_t1
#define tx_table_s2_t2 s2_t2

#define tx_table_s3_t0 s3_t0
#define tx_table_s3_t1 s3_t1
#define tx_table_s3_t2 s3_t2

#define tx_table_s4_t0 s4_t0
#define tx_table_s4_t1 s4_t1
#define tx_table_s4_t2 s4_t2

#define tx_table_s5_t0 s5_t0
#define tx_table_s5_t1 s5_t1
#define tx_table_s5_t2 s5_t2

#define tx_table_s6_t0 s6_t0
#define tx_table_s6_t1 s6_t1
#define tx_table_s6_t2 s6_t2

#define tx_table_s7_t0 s7_t0
#define tx_table_s7_t1 s7_t1
#define tx_table_s7_t2 s7_t2

/**** action declarations ****/

#define tx_table_s0_t0_action nvme_req_tx_sqcb_process

#define tx_table_s1_t0_action nvme_req_tx_sqe_process

#define tx_table_s2_t0_action nvme_req_tx_nscb_process

#define tx_table_s3_t0_action nvme_req_tx_sess_bitmap0_process
#define tx_table_s3_t1_action nvme_req_tx_sess_bitmap1_process
#define tx_table_s3_t2_action nvme_req_tx_sqe_prp_process

#define tx_table_s4_t0_action nvme_req_tx_sess_select_process
#define tx_table_s4_t1_action nvme_req_tx_resourcecb_process

#define tx_table_s5_t0_action nvme_req_tx_sessprodcb_process
#define tx_table_s5_t1_action nvme_req_tx_cmdid_fetch_process

#define tx_table_s6_t0_action nvme_req_tx_sqcb_writeback_process

#define tx_table_s7_t1_action nvme_req_tx_sq_statscb_process

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

header_type nvme_req_tx_to_stage_sqe_info_t {
    fields {
        lif_ns_start                     :  16;
        pad                              :  112;
    }
}

header_type nvme_req_tx_to_stage_nscb_info_t {
    fields {
        log_host_page_size  :    5;
        rsvd0               :    3;
        opc                 :    8;
        slba                :   64;
        nlb                 :   16;
        pad                 :   32;
    }
}

header_type nvme_req_tx_to_stage_sesscb_bitmap_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_req_tx_to_stage_sess_select_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_req_tx_to_stage_sessprodcb_info_t {
    fields {
        prp1_dma_valid                   : 1;
        prp2_dma_valid                   : 1;
        prp3_dma_valid                   : 1;
        rsvd0                            : 5;
        prp1_dma_bytes                   : 8;
        prp2_dma_bytes                   : 16;
        prp3_dma_bytes                   : 16;
        pad                              : 80;
    }
}

header_type nvme_req_tx_to_stage_sqcb_writeback_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_req_tx_to_stage_sq_statscb_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_req_tx_sqcb_to_sqe_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_req_tx_sqe_to_nscb_t {
    fields {
        dptr1               :   64;
        dptr2               :   64;
        pad                 :   32;
    }
}

header_type nvme_req_tx_nscb_to_sess_bitmap_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_req_tx_nscb_to_sqe_prp_t {
    fields {
        prp3_dma_bytes                   : 16;
        pad                 : 144;
    }
}

header_type nvme_req_tx_sess_bitmap_to_sess_select_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_req_tx_sess_bitmap_to_resourcecb_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_req_tx_sess_select_to_sessprodcb_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_req_tx_resourcecb_to_cmdid_fetch_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_req_tx_sessprodcb_to_sqcb_writeback_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_req_tx_sqcb_writeback_to_sq_statscb_t {
    fields {
        pad                 : 160;
    }
}

/**** scratch for D-vectors ****/

@pragma scratch_metadata
metadata sqcb_t sqcb_d;

@pragma scratch_metadata
metadata nvme_sqe_t nvme_sqe_d;

@pragma scratch_metadata
metadata nscb_t nscb_d;

@pragma scratch_metadata
metadata ptr64_t sqe_prp_list_d;

@pragma scratch_metadata
metadata sessprodcb_t sessprodcb_d;

@pragma scratch_metadata
metadata resourcecb_t resourcecb_d;

@pragma scratch_metadata
metadata cmdid_ring_entry_t cmdid_ring_entry_d;

@pragma scratch_metadata
metadata sq_statscb_t sq_statscb_d;

/**** global header unions ****/

@pragma pa_header_union ingress common_global
metadata phv_global_common_t phv_global_common;
@pragma scratch_metadata
metadata phv_global_common_t phv_global_common_scr;

/**** to stage header unions ****/

//To-Stage-0

//To-Stage-1
@pragma pa_header_union ingress to_stage_1
metadata nvme_req_tx_to_stage_sqe_info_t to_s1_info;
@pragma scratch_metadata
metadata nvme_req_tx_to_stage_sqe_info_t to_s1_info_scr;

//To-Stage-2
@pragma pa_header_union ingress to_stage_2
metadata nvme_req_tx_to_stage_nscb_info_t to_s2_info;
@pragma scratch_metadata
metadata nvme_req_tx_to_stage_nscb_info_t to_s2_info_scr;

//To-Stage-3
@pragma pa_header_union ingress to_stage_3
metadata nvme_req_tx_to_stage_sesscb_bitmap_info_t to_s3_info;
@pragma scratch_metadata
metadata nvme_req_tx_to_stage_sesscb_bitmap_info_t to_s3_info_scr;

//To-Stage-4
@pragma pa_header_union ingress to_stage_4
metadata nvme_req_tx_to_stage_sess_select_info_t to_s4_info;
@pragma scratch_metadata
metadata nvme_req_tx_to_stage_sess_select_info_t to_s4_info_scr;

//To-Stage-5
@pragma pa_header_union ingress to_stage_5
metadata nvme_req_tx_to_stage_sessprodcb_info_t to_s5_info;
@pragma scratch_metadata
metadata nvme_req_tx_to_stage_sessprodcb_info_t to_s5_info_scr;

//To-Stage-6
@pragma pa_header_union ingress to_stage_6
metadata nvme_req_tx_to_stage_sqcb_writeback_info_t to_s6_info;
@pragma scratch_metadata
metadata nvme_req_tx_to_stage_sqcb_writeback_info_t to_s6_info_scr;

//To-Stage-7
@pragma pa_header_union ingress to_stage_7
metadata nvme_req_tx_to_stage_sq_statscb_info_t to_s7_info;
@pragma scratch_metadata
metadata nvme_req_tx_to_stage_sq_statscb_info_t to_s7_info_scr;


/**** stage to stage header unions ****/

//Table-0
@pragma pa_header_union ingress common_t0_s2s t0_s2s_sqcb_to_sqe_info t0_s2s_sqe_to_nscb_info t0_s2s_nscb_to_sess_bitmap_info t0_s2s_sess_bitmap_to_sess_select_info t0_s2s_sess_select_to_sessprodcb_info t0_s2s_sessprodcb_to_sqcb_writeback_info
metadata nvme_req_tx_sqcb_to_sqe_t t0_s2s_sqcb_to_sqe_info;
@pragma scratch_metadata
metadata nvme_req_tx_sqcb_to_sqe_t t0_s2s_sqcb_to_sqe_info_scr;

metadata nvme_req_tx_sqe_to_nscb_t t0_s2s_sqe_to_nscb_info;
@pragma scratch_metadata
metadata nvme_req_tx_sqe_to_nscb_t t0_s2s_sqe_to_nscb_info_scr;

metadata nvme_req_tx_nscb_to_sess_bitmap_t t0_s2s_nscb_to_sess_bitmap_info;
@pragma scratch_metadata
metadata nvme_req_tx_nscb_to_sess_bitmap_t t0_s2s_nscb_to_sess_bitmap_info_scr;

metadata nvme_req_tx_sess_bitmap_to_sess_select_t t0_s2s_sess_bitmap_to_sess_select_info;
@pragma scratch_metadata
metadata nvme_req_tx_sess_bitmap_to_sess_select_t t0_s2s_sess_bitmap_to_sess_select_info_scr;

metadata nvme_req_tx_sess_select_to_sessprodcb_t t0_s2s_sess_select_to_sessprodcb_info;
@pragma scratch_metadata
metadata nvme_req_tx_sess_select_to_sessprodcb_t t0_s2s_sess_select_to_sessprodcb_info_scr;

metadata nvme_req_tx_sessprodcb_to_sqcb_writeback_t t0_s2s_sessprodcb_to_sqcb_writeback_info;
@pragma scratch_metadata
metadata nvme_req_tx_sessprodcb_to_sqcb_writeback_t t0_s2s_sessprodcb_to_sqcb_writeback_info_scr;

//Table-1
@pragma pa_header_union ingress common_t1_s2s t1_s2s_sqcb_writeback_to_sq_statscb_info t1_s2s_nscb_to_sess_bitmap_info t1_s2s_sess_bitmap_to_resourcecb_info t1_s2s_resourcecb_to_cmdid_fetch_info

metadata nvme_req_tx_sqcb_writeback_to_sq_statscb_t t1_s2s_sqcb_writeback_to_sq_statscb_info;
@pragma scratch_metadata
metadata nvme_req_tx_sqcb_writeback_to_sq_statscb_t t1_s2s_sqcb_writeback_to_sq_statscb_info_scr;

metadata nvme_req_tx_nscb_to_sess_bitmap_t t1_s2s_nscb_to_sess_bitmap_info;
@pragma scratch_metadata
metadata nvme_req_tx_nscb_to_sess_bitmap_t t1_s2s_nscb_to_sess_bitmap_info_scr;

metadata nvme_req_tx_sess_bitmap_to_resourcecb_t t1_s2s_sess_bitmap_to_resourcecb_info;
@pragma scratch_metadata
metadata nvme_req_tx_sess_bitmap_to_resourcecb_t t1_s2s_sess_bitmap_to_resourcecb_info_scr;

metadata nvme_req_tx_resourcecb_to_cmdid_fetch_t t1_s2s_resourcecb_to_cmdid_fetch_info;
@pragma scratch_metadata
metadata nvme_req_tx_resourcecb_to_cmdid_fetch_t t1_s2s_resourcecb_to_cmdid_fetch_info_scr;

//Table-2
@pragma pa_header_union ingress common_t2_s2s t2_s2s_nscb_to_sqe_prp_info 

metadata nvme_req_tx_nscb_to_sqe_prp_t t2_s2s_nscb_to_sqe_prp_info;
@pragma scratch_metadata
metadata nvme_req_tx_nscb_to_sqe_prp_t t2_s2s_nscb_to_sqe_prp_info_scr;

/**** PHV Layout ****/
@pragma dont_trim
metadata cmd_context_t cmd_ctxt;
@pragma dont_trim
metadata data64_t session_db;
@pragma dont_trim
metadata index16_t data_cindex;
@pragma dont_trim
metadata index16_t aol_cindex;
@pragma dont_trim
metadata index16_t cmdid_cindex;
@pragma dont_trim
metadata sess_wqe_t sess_wqe;
@pragma dont_trim
metadata ptr64_t prp1;
@pragma dont_trim
metadata ptr64_t prp2;

@pragma pa_align 128
@pragma dont_trim
metadata dma_cmd_phv2mem_t wqe_dma;             //dma cmd 0
@pragma dont_trim
metadata dma_cmd_phv2mem_t prp1_dma;            //dma cmd 1
@pragma dont_trim
metadata dma_cmd_mem2mem_t prp2_src_dma;        //dma cmd 2
@pragma dont_trim
metadata dma_cmd_mem2mem_t prp2_dst_dma;        //dma cmd 3
@pragma dont_trim
metadata dma_cmd_mem2mem_t prp3_src_dma;        //dma cmd 4
@pragma dont_trim
metadata dma_cmd_mem2mem_t prp3_dst_dma;        //dma cmd 5
@pragma dont_trim
metadata dma_cmd_mem2mem_t data_page1_src_dma;  //dma cmd 6
@pragma dont_trim
metadata dma_cmd_mem2mem_t data_page1_dst_dma;  //dma cmd 7
@pragma dont_trim
metadata dma_cmd_mem2mem_t data_page2_src_dma;  //dma cmd 8
@pragma dont_trim
metadata dma_cmd_mem2mem_t data_page2_dst_dma;  //dma cmd 9
@pragma dont_trim
metadata dma_cmd_mem2mem_t aol_desc1_src_dma;   //dma cmd 10
@pragma dont_trim
metadata dma_cmd_mem2mem_t aol_desc1_dst_dma;   //dma cmd 11
@pragma dont_trim
metadata dma_cmd_mem2mem_t aol_desc2_src_dma;   //dma cmd 12
@pragma dont_trim
metadata dma_cmd_mem2mem_t aol_desc2_dst_dma;   //dma cmd 13
@pragma dont_trim
metadata dma_cmd_phv2mem_t data_cindex_dma;     //dma cmd 14
@pragma dont_trim
metadata dma_cmd_phv2mem_t aol_cindex_dma;      //dma cmd 15
@pragma dont_trim
metadata dma_cmd_phv2mem_t cmdid_cindex_dma;    //dma cmd 16
@pragma dont_trim
metadata dma_cmd_phv2mem_t session_wqe_dma;     //dma cmd 17
@pragma dont_trim
metadata dma_cmd_phv2mem_t session_db_dma;      //dma cmd 18


/*
 * Stage 0 table 0 action
 */
action nvme_req_tx_sqcb_process (SQCB_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage

    // D-vector
    GENERATE_SQCB_D
}

action nvme_req_tx_sqe_process (NVME_SQE_PARAMS) {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_info_scr.lif_ns_start, to_s1_info.lif_ns_start);
    modify_field(to_s1_info_scr.pad, to_s1_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_sqcb_to_sqe_info_scr.pad, t0_s2s_sqcb_to_sqe_info.pad);

    // D-vector
    GENERATE_NVME_SQE_D
}

action nvme_req_tx_nscb_process (NSCB_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s2_info_scr.rsvd0, to_s2_info.rsvd0);
    modify_field(to_s2_info_scr.opc, to_s2_info.opc);
    modify_field(to_s2_info_scr.slba, to_s2_info.slba);
    modify_field(to_s2_info_scr.nlb, to_s2_info.nlb);
    modify_field(to_s2_info_scr.pad, to_s2_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_sqe_to_nscb_info_scr.dptr1, t0_s2s_sqe_to_nscb_info.dptr1);
    modify_field(t0_s2s_sqe_to_nscb_info_scr.dptr2, t0_s2s_sqe_to_nscb_info.dptr2);
    modify_field(t0_s2s_sqe_to_nscb_info_scr.pad, t0_s2s_sqe_to_nscb_info.pad);

    // D-vector
    GENERATE_NSCB_D
}

action nvme_req_tx_sqe_prp_process (SQE_PRP_LIST_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s3_info_scr.pad, to_s3_info.pad);
    
    // stage to stage
    modify_field(t2_s2s_nscb_to_sqe_prp_info_scr.pad, t2_s2s_nscb_to_sqe_prp_info.pad);

    // D-vector
    GENERATE_SQE_PRP_LIST_D
}


action nvme_req_tx_sess_bitmap0_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s3_info_scr.pad, to_s3_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_nscb_to_sess_bitmap_info_scr.pad, t0_s2s_nscb_to_sess_bitmap_info.pad);
}

action nvme_req_tx_sess_bitmap1_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s3_info_scr.pad, to_s3_info.pad);
    
    // stage to stage
    modify_field(t1_s2s_nscb_to_sess_bitmap_info_scr.pad, t1_s2s_nscb_to_sess_bitmap_info.pad);
}

action nvme_req_tx_sess_select_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_info_scr.pad, to_s4_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_sess_bitmap_to_sess_select_info_scr.pad, t0_s2s_sess_bitmap_to_sess_select_info.pad);
}

action nvme_req_tx_resourcecb_process (RESOURCECB_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_info_scr.pad, to_s4_info.pad);
    
    // stage to stage
    modify_field(t1_s2s_sess_bitmap_to_resourcecb_info_scr.pad, t1_s2s_sess_bitmap_to_resourcecb_info.pad);

    // D-vector
    GENERATE_RESOURCECB_D
}


action nvme_req_tx_sessprodcb_process (SESSPRODCB_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s5_info_scr.prp1_dma_valid, to_s5_info.prp1_dma_valid);
    modify_field(to_s5_info_scr.prp2_dma_valid, to_s5_info.prp2_dma_valid);
    modify_field(to_s5_info_scr.prp3_dma_valid, to_s5_info.prp3_dma_valid);
    modify_field(to_s5_info_scr.rsvd0, to_s5_info.rsvd0);
    modify_field(to_s5_info_scr.prp1_dma_bytes, to_s5_info.prp1_dma_bytes);
    modify_field(to_s5_info_scr.prp2_dma_bytes, to_s5_info.prp2_dma_bytes);
    modify_field(to_s5_info_scr.prp3_dma_bytes, to_s5_info.prp3_dma_bytes);
    modify_field(to_s5_info_scr.pad, to_s5_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_sess_select_to_sessprodcb_info_scr.pad, t0_s2s_sess_select_to_sessprodcb_info.pad);

    // D-vector
    GENERATE_SESSPRODCB_D
}

action nvme_req_tx_cmdid_fetch_process (CMDID_RING_ENTRY_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s5_info_scr.prp1_dma_valid, to_s5_info.prp1_dma_valid);
    modify_field(to_s5_info_scr.prp2_dma_valid, to_s5_info.prp2_dma_valid);
    modify_field(to_s5_info_scr.prp3_dma_valid, to_s5_info.prp3_dma_valid);
    modify_field(to_s5_info_scr.rsvd0, to_s5_info.rsvd0);
    modify_field(to_s5_info_scr.prp1_dma_bytes, to_s5_info.prp1_dma_bytes);
    modify_field(to_s5_info_scr.prp2_dma_bytes, to_s5_info.prp2_dma_bytes);
    modify_field(to_s5_info_scr.prp3_dma_bytes, to_s5_info.prp3_dma_bytes);
    modify_field(to_s5_info_scr.pad, to_s5_info.pad);
    
    // stage to stage
    modify_field(t1_s2s_resourcecb_to_cmdid_fetch_info_scr.pad, t1_s2s_resourcecb_to_cmdid_fetch_info.pad);

    // D-vector
    GENERATE_CMDID_RING_ENTRY_D
}

action nvme_req_tx_sqcb_writeback_process (SQCB_PARAMS_NON_STG0) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s6_info_scr.pad, to_s6_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_sessprodcb_to_sqcb_writeback_info_scr.pad, t0_s2s_sessprodcb_to_sqcb_writeback_info.pad);

    // D-vector
    GENERATE_SQCB_D_NON_STG0
}

action nvme_req_tx_sq_statscb_process (SQ_STATSCB_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s7_info_scr.pad, to_s7_info.pad);
    
    // stage to stage
    modify_field(t1_s2s_sqcb_writeback_to_sq_statscb_info_scr.pad, t1_s2s_sqcb_writeback_to_sq_statscb_info.pad);

    // D-vector
    GENERATE_SQ_STATSCB_D
}
