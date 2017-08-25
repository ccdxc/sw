#ifndef __RESP_RX_H
#define __RESP_RX_H
#include "capri.h"
#include "types.h"
#include "resp_rx_args.h"
#include "INGRESS_p.h"
#include "common_phv.h"

#define RESP_RX_MAX_DMA_CMDS        16
#define RESP_RX_DMA_CMD_PYLD_BASE   0

#define RESP_RX_DMA_CMD_CQ          (RESP_RX_MAX_DMA_CMDS - 3)

#define RESP_RX_DMA_CMD_START_FLIT_ID   8 // flits 8-11 are used for dma cmds

// phv 
struct resp_rx_phv_t {
    // dma commands (flit 8 - 11)

    // scratch (flit 6 - 7)
    eq_int_num: 16;
    struct eqwqe_t eqwqe;
    struct cqwqe_t cqwqe;
    struct ack_info_t ack_info;
    my_token_id: 8;

    // common rx (flit 0 - 5)
    struct phv_ common;
};

struct resp_rx_phv_global_t {
    struct phv_global_common_t common;
};

// stage to stage argument structures

struct resp_rx_s0_t {
    lif: 11;
    qtype: 3;
    qid: 24;
    struct p4_2_p4plus_app_hdr_t app_hdr;
};

struct resp_rx_rqcb_process_k_old_t {
    struct capri_intrinsic_s0_k_t intrinsic;
    struct resp_rx_s0_t args;
};

struct resp_rx_rqcb_process_k_t {
    _pad_0 : 8;    // k[511:504]
    p4_rxdma_intr_dma_cmd_ptr : 6;    // k[503:498]
    p4_rxdma_intr_qstate_addr : 34;    // k[497:464]
    app_data0_1 : 88;    // k[463:376]
    app_data1   : 88;    // k[375:288]
    _pad_5 : 32;    // k[287:256]


    p4_intr_global_tm_iq : 5;    // k[255:251]
    lif          : 11;    // k[250:240]
    qid          : 24;    // k[239:216]
    qtype        : 3;    // k[215:213]
    rx_splitter_offsets0_e4 : 5;    // k[212:208]
    app_type     : 4;    // k[207:204]
    table0_valid : 1;    // k[203:203]
    table1_valid : 1;    // k[202:202]
    table2_valid : 1;    // k[201:201]
    table3_valid : 1;    // k[200:200]
                     //app_data0_0  : 160;    // k[199:40]
    struct app_data0_0_t app_data0_0;
    _pad_18      : 40;    // k[39:0]
};

struct resp_rx_rqcb_to_pt_info_t {
    in_progress: 1;
    page_seg_offset: 3;
    tbl_id: 3;
    cache: 1;
    page_offset: 16;
    remaining_payload_bytes: 16;
};

struct resp_rx_rqpt_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct phv_global_common_t global;
    struct resp_rx_rqcb_to_pt_info_t args;
};

struct resp_rx_rqcb_to_wqe_info_t {
    //rqcb1
    in_progress:1;
    cache:1;
    remaining_payload_bytes: 16;
    //rqcb2
    curr_wqe_ptr: 64;
    current_sge_id: 8;
    current_sge_offset: 32;
    //computed
    num_valid_sges: 8;
};

struct resp_rx_rqwqe_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct phv_global_common_t global;
    struct resp_rx_rqcb_to_wqe_info_t args;
};

struct resp_rx_sge_to_lkey_info_t {
    sge_va: 64;
    sge_bytes: 16;
    page_size: 16;
    dma_cmd_start_index: 8;
    key_id: 8;
    sge_index: 8;
    cq_dma_cmd_index: 8;
    cq_id:24;
    dma_cmdeop: 1;
    rsvd: 7;
};

struct resp_rx_rqlkey_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct phv_global_common_t global;
    struct resp_rx_sge_to_lkey_info_t args;
};

struct resp_rx_rqcb0_write_back_info_t {
    in_progress: 8;
    incr_nxt_to_go_token_id: 1;
    incr_c_index: 1;
    tbl_id: 3;
    cache: 1;
};

struct resp_rx_rqcb0_write_back_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct phv_global_common_t global;
    struct resp_rx_rqcb0_write_back_info_t args;
};

struct resp_rx_rqcb1_write_back_info_t {
    curr_wqe_ptr: 64;
    current_sge_offset: 32;
    current_sge_id: 8;
    update_num_sges: 1;
    num_sges: 8;
};

struct resp_rx_rqcb1_write_back_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct phv_global_common_t global;
    struct resp_rx_rqcb1_write_back_info_t args;
};

struct resp_rx_lkey_to_pt_info_t {
    pt_offset: 32;
    pt_bytes: 16;
    dma_cmd_start_index: 8;
    sge_index: 8;
    log_page_size: 5;
    dma_cmdeop: 1;
    rsvd: 2;
};

struct resp_rx_ptseg_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct phv_global_common_t global;
    struct resp_rx_lkey_to_pt_info_t args;
};

struct resp_rx_compl_or_inv_rkey_info_t {
    r_key:  32;
    dma_cmd_index: 8;
    tbl_id: 3;
    rsvd: 5;
};

struct resp_rx_compl_or_inv_rkey_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct phv_global_common_t global;
    struct resp_rx_compl_or_inv_rkey_info_t args;
};

struct resp_rx_rqcb_to_cq_info_t {
    tbl_id: 3;
    rsvd: 5;
    dma_cmd_index: 8;
};

struct resp_rx_cqcb_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct phv_global_common_t global;
    struct resp_rx_rqcb_to_cq_info_t args;
};

struct resp_rx_cqcb_to_pt_info_t {
    page_offset: 16;
    page_seg_offset: 8;
    dma_cmd_index: 8;
    eq_id: 24;
    cq_id: 24;
    arm: 1;
    tbl_id: 3;
    rsvd: 4;
};

struct resp_rx_cqpt_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct phv_global_common_t global;
    struct resp_rx_cqcb_to_pt_info_t args;
};

struct resp_rx_cqcb_to_eq_info_t {
    tbl_id: 3;
    rsvd: 5;
    dma_cmd_index: 8;  
    cq_id: 24;
};

struct resp_rx_eqcb_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct phv_global_common_t global;
    struct resp_rx_cqcb_to_eq_info_t args;
};


#endif //__RESP_RX_H
