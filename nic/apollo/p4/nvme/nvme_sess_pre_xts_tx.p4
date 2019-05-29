/***********************************************************************/
/* nvme_sess_pre_xts_tx.p4 */
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

#define tx_table_s0_t0_action nvme_sessprexts_tx_cb_process

#define tx_table_s1_t0_action nvme_sessprexts_tx_sess_wqe_process
#define tx_table_s1_t2_action nvme_sessprexts_tx_nmdpr_process

#define tx_table_s2_t0_action nvme_sessprexts_tx_pdu_ctxt_process
#define tx_table_s2_t1_action nvme_sessprexts_tx_xtscb_process
#define tx_table_s2_t2_action nvme_sessprexts_tx_page_fetch_process

#define tx_table_s3_t0_action nvme_sessprexts_tx_cb_writeback_process

#define tx_table_s4_t0_action nvme_sessprexts_tx_ip_desc_process
#define tx_table_s4_t1_action nvme_sessprexts_tx_op_desc_process

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

header_type nvme_sessprexts_tx_to_stage_sess_wqe_info_t {
    fields {
        pad                              :  128;
    }
}

header_type nvme_sessprexts_tx_to_stage_pdu_ctxt_info_t {
    fields {
        incr_num_pages                   :    1;
        pad                              :  127;
    }
}

header_type nvme_sessprexts_tx_to_stage_writeback_info_t {
    fields {
        page_ptr                         :   64;
        pdu_ctxt_ptr                     :   64;
    }
}

header_type nvme_sessprexts_tx_to_stage_ip_op_desc_info_t {
    fields {
        prp1_offset                      :   32;
        prp1_bytes                       :   32;
        prp2_bytes                       :   32;
        prp2_valid                       :    1;
        pad                              :   31;
    }
}

header_type nvme_sessprexts_tx_cb_to_sess_wqe_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_sessprexts_tx_cb_to_nmdpr_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_sessprexts_tx_sess_wqe_to_pdu_ctxt_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_sessprexts_tx_sess_wqe_to_xtscb_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_sessprexts_tx_nmdpr_to_page_fetch_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_sessprexts_tx_pdu_ctxt_to_writeback_t {
    fields {
        slba                :  64;
        nlb                 :  16;
        log_lba_size        :   5;
        log_host_page_size  :   5;
        prp1_offset         :  16;
        pad                 :  54;
    }
}

header_type nvme_sessprexts_tx_writeback_to_ip_desc_t {
    fields {
        pad                 : 160;
    }
}

header_type nvme_sessprexts_tx_writeback_to_op_desc_t {
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
metadata nmdpr_t nmdpr_d;

@pragma scratch_metadata
metadata xtscb_t xtscb_d;

@pragma scratch_metadata
metadata nmdpr_page_t nmdpr_page_d;

@pragma scratch_metadata
metadata pdu_context0_t pdu_ctxt0_d;

@pragma scratch_metadata
metadata prp_pair_t prp_pair_d;

/**** global header unions ****/

@pragma pa_header_union ingress common_global
metadata phv_global_common_t phv_global_common;
@pragma scratch_metadata
metadata phv_global_common_t phv_global_common_scr;

/**** to stage header unions ****/

//To-Stage-0

//To-Stage-1
@pragma pa_header_union ingress to_stage_1
metadata nvme_sessprexts_tx_to_stage_sess_wqe_info_t to_s1_info;
@pragma scratch_metadata
metadata nvme_sessprexts_tx_to_stage_sess_wqe_info_t to_s1_info_scr;

//To-Stage-2
@pragma pa_header_union ingress to_stage_2
metadata nvme_sessprexts_tx_to_stage_pdu_ctxt_info_t to_s2_info;
@pragma scratch_metadata
metadata nvme_sessprexts_tx_to_stage_pdu_ctxt_info_t to_s2_info_scr;

//To-Stage-3
@pragma pa_header_union ingress to_stage_3
metadata nvme_sessprexts_tx_to_stage_writeback_info_t to_s3_info;
@pragma scratch_metadata
metadata nvme_sessprexts_tx_to_stage_writeback_info_t to_s3_info_scr;

//To-Stage-4
@pragma pa_header_union ingress to_stage_4
metadata nvme_sessprexts_tx_to_stage_ip_op_desc_info_t to_s4_info;
@pragma scratch_metadata
metadata nvme_sessprexts_tx_to_stage_ip_op_desc_info_t to_s4_info_scr;

/**** stage to stage header unions ****/

//Table-0
@pragma pa_header_union ingress common_t0_s2s t0_s2s_cb_to_sess_wqe_info t0_s2s_sess_wqe_to_pdu_ctxt_info t0_s2s_pdu_ctxt_to_writeback_info t0_s2s_writeback_to_ip_desc_info

metadata nvme_sessprexts_tx_cb_to_sess_wqe_t t0_s2s_cb_to_sess_wqe_info;
@pragma scratch_metadata
metadata nvme_sessprexts_tx_cb_to_sess_wqe_t t0_s2s_cb_to_sess_wqe_info_scr;

metadata nvme_sessprexts_tx_sess_wqe_to_pdu_ctxt_t t0_s2s_sess_wqe_to_pdu_ctxt_info;
@pragma scratch_metadata
metadata nvme_sessprexts_tx_sess_wqe_to_pdu_ctxt_t t0_s2s_sess_wqe_to_pdu_ctxt_info_scr;

metadata nvme_sessprexts_tx_pdu_ctxt_to_writeback_t t0_s2s_pdu_ctxt_to_writeback_info;
@pragma scratch_metadata
metadata nvme_sessprexts_tx_pdu_ctxt_to_writeback_t t0_s2s_pdu_ctxt_to_writeback_info_scr;

metadata nvme_sessprexts_tx_writeback_to_ip_desc_t t0_s2s_writeback_to_ip_desc_info;
@pragma scratch_metadata
metadata nvme_sessprexts_tx_writeback_to_ip_desc_t t0_s2s_writeback_to_ip_desc_info_scr;

//Table-1
@pragma pa_header_union ingress common_t1_s2s t1_s2s_sess_wqe_to_xtscb_info t1_s2s_writeback_to_op_desc_info

metadata nvme_sessprexts_tx_sess_wqe_to_xtscb_t t1_s2s_sess_wqe_to_xtscb_info;
@pragma scratch_metadata
metadata nvme_sessprexts_tx_sess_wqe_to_xtscb_t t1_s2s_sess_wqe_to_xtscb_info_scr;

metadata nvme_sessprexts_tx_writeback_to_op_desc_t t1_s2s_writeback_to_op_desc_info;
@pragma scratch_metadata
metadata nvme_sessprexts_tx_writeback_to_op_desc_t t1_s2s_writeback_to_op_desc_info_scr;


//Table-2
@pragma pa_header_union ingress common_t2_s2s t2_s2s_cb_to_nmdpr_info t2_s2s_nmdpr_to_page_fetch_info

metadata nvme_sessprexts_tx_cb_to_nmdpr_t t2_s2s_cb_to_nmdpr_info;
@pragma scratch_metadata
metadata nvme_sessprexts_tx_cb_to_nmdpr_t t2_s2s_cb_to_nmdpr_info_scr;

metadata nvme_sessprexts_tx_nmdpr_to_page_fetch_t t2_s2s_nmdpr_to_page_fetch_info;
@pragma scratch_metadata
metadata nvme_sessprexts_tx_nmdpr_to_page_fetch_t t2_s2s_nmdpr_to_page_fetch_info_scr;


/**** PHV Layout ****/
@pragma dont_trim
metadata pkt_desc_one_aol_t pkt_desc_one_aol;
@pragma dont_trim
metadata ptr64_t page_ptr;
@pragma dont_trim
metadata index32_t xts_db;
@pragma dont_trim
metadata xts_aol_desc_t ip_desc;
@pragma dont_trim
metadata xts_aol_desc_t op_desc;
@pragma dont_trim
metadata xts_desc_t xts_desc;
@pragma dont_trim
metadata xts_iv_t iv;
@pragma dont_trim
metadata dgst_one_aol_t dgst_one_aol;

@pragma pa_align 128
@pragma dont_trim
metadata dma_cmd_phv2mem_t pkt_desc_dma;        //dma cmd 0
@pragma dont_trim
metadata dma_cmd_phv2mem_t page_ptr_dma;        //dma cmd 1
@pragma dont_trim
metadata dma_cmd_phv2mem_t ip_desc_dma;         //dma cmd 2
@pragma dont_trim
metadata dma_cmd_phv2mem_t op_desc_dma;         //dma cmd 3
@pragma dont_trim
metadata dma_cmd_phv2mem_t iv_dma;              //dma cmd 4
@pragma dont_trim
metadata dma_cmd_phv2mem_t xts_desc_dma;        //dma cmd 5
@pragma dont_trim
metadata dma_cmd_phv2mem_t dgst_one_aol_dma;    //dma cmd 6
@pragma dont_trim
metadata dma_cmd_phv2mem_t xts_db_dma;          //dma cmd 7

/*
 * Stage 0 table 0 action
 */
action nvme_sessprexts_tx_cb_process (SESSXTSTXCB_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage

    // D-vector
    GENERATE_SESSXTSTXCB_D
}

action nvme_sessprexts_tx_sess_wqe_process (SESS_WQE_PARAMS) {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_info_scr.pad, to_s1_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_cb_to_sess_wqe_info_scr.pad, t0_s2s_cb_to_sess_wqe_info.pad);

    // D-vector
    GENERATE_SESS_WQE_D
}

action nvme_sessprexts_tx_nmdpr_process (NMDPR_PARAMS) {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_info_scr.pad, to_s1_info.pad);
    
    // stage to stage
    modify_field(t2_s2s_cb_to_nmdpr_info_scr.pad, t2_s2s_cb_to_nmdpr_info.pad);

    // D-vector
    GENERATE_NMDPR_D
}

action nvme_sessprexts_tx_pdu_ctxt_process (PDU_CTXT0_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s2_info_scr.incr_num_pages, to_s2_info.incr_num_pages);
    modify_field(to_s2_info_scr.pad, to_s2_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_sess_wqe_to_pdu_ctxt_info_scr.pad, t0_s2s_sess_wqe_to_pdu_ctxt_info.pad);

    // D-vector
    GENERATE_PDU_CTXT0_D
}

action nvme_sessprexts_tx_xtscb_process (XTSCB_PARAMS) {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s2_info_scr.incr_num_pages, to_s2_info.incr_num_pages);
    modify_field(to_s2_info_scr.pad, to_s2_info.pad);
    
    // stage to stage
    modify_field(t1_s2s_sess_wqe_to_xtscb_info_scr.pad, t1_s2s_sess_wqe_to_xtscb_info.pad);

    // D-vector
    GENERATE_XTSCB_D
}

action nvme_sessprexts_tx_page_fetch_process (NMDPR_PAGE_PARAMS) {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s2_info_scr.incr_num_pages, to_s2_info.incr_num_pages);
    modify_field(to_s2_info_scr.pad, to_s2_info.pad);
    
    // stage to stage
    modify_field(t2_s2s_nmdpr_to_page_fetch_info_scr.pad, t2_s2s_nmdpr_to_page_fetch_info.pad);

    // D-vector
    GENERATE_NMDPR_PAGE_D
}

action nvme_sessprexts_tx_cb_writeback_process (SESSXTSTXCB_PARAMS_NON_STG0) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s3_info_scr.page_ptr, to_s3_info.page_ptr);
    modify_field(to_s3_info_scr.pdu_ctxt_ptr, to_s3_info.pdu_ctxt_ptr);
    
    // stage to stage
    modify_field(t0_s2s_pdu_ctxt_to_writeback_info_scr.slba, t0_s2s_pdu_ctxt_to_writeback_info.slba);
    modify_field(t0_s2s_pdu_ctxt_to_writeback_info_scr.nlb, t0_s2s_pdu_ctxt_to_writeback_info.nlb);
    modify_field(t0_s2s_pdu_ctxt_to_writeback_info_scr.log_lba_size, t0_s2s_pdu_ctxt_to_writeback_info.log_lba_size);
    modify_field(t0_s2s_pdu_ctxt_to_writeback_info_scr.log_host_page_size, t0_s2s_pdu_ctxt_to_writeback_info.log_host_page_size);
    modify_field(t0_s2s_pdu_ctxt_to_writeback_info_scr.prp1_offset, t0_s2s_pdu_ctxt_to_writeback_info.prp1_offset);
    modify_field(t0_s2s_pdu_ctxt_to_writeback_info_scr.pad, t0_s2s_pdu_ctxt_to_writeback_info.pad);

    // D-vector
    GENERATE_SESSXTSTXCB_D_NON_STG0
}


action nvme_sessprexts_tx_ip_desc_process (PRP_PAIR_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_info_scr.prp1_offset, to_s4_info.prp1_offset);
    modify_field(to_s4_info_scr.prp1_bytes, to_s4_info.prp1_bytes);
    modify_field(to_s4_info_scr.prp2_bytes, to_s4_info.prp2_bytes);
    modify_field(to_s4_info_scr.prp2_valid, to_s4_info.prp2_valid);
    modify_field(to_s4_info_scr.pad, to_s4_info.pad);
    
    // stage to stage
    modify_field(t0_s2s_writeback_to_ip_desc_info_scr.pad, t0_s2s_writeback_to_ip_desc_info.pad);

    // D-vector
    GENERATE_PRP_PAIR_D
}

action nvme_sessprexts_tx_op_desc_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_info_scr.prp1_offset, to_s4_info.prp1_offset);
    modify_field(to_s4_info_scr.prp1_bytes, to_s4_info.prp1_bytes);
    modify_field(to_s4_info_scr.prp2_bytes, to_s4_info.prp2_bytes);
    modify_field(to_s4_info_scr.prp2_valid, to_s4_info.prp2_valid);
    modify_field(to_s4_info_scr.pad, to_s4_info.pad);
    
    // stage to stage
    modify_field(t1_s2s_writeback_to_op_desc_info_scr.pad, t1_s2s_writeback_to_op_desc_info.pad);
}
