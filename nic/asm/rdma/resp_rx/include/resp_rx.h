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

struct resp_rx_rqcb_to_pt_info_t {
    in_progress: 1;
    page_seg_offset: 3;
    tbl_id: 3;
    cache: 1;
    page_offset: 16;
    remaining_payload_bytes: 16;
    inv_r_key: 32;
    pad: 88;
};

struct resp_rx_rqpt_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_rx_rqcb_to_pt_info_t args;
    struct phv_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct resp_rx_rqcb_to_wqe_info_t {
    //rqcb1
    in_progress:1;
    cache:1;
    current_sge_id: 6;
    remaining_payload_bytes: 16;
    //rqcb2
    curr_wqe_ptr: 64;
    inv_r_key: 32;
    current_sge_offset: 32;
    //computed
    num_valid_sges: 8;
};

struct resp_rx_rqwqe_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_rx_rqcb_to_wqe_info_t args;
    struct phv_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct resp_rx_key_info_t {
    va: 64;
    len: 16;
    acc_ctrl: 8;
    dma_cmd_start_index: 8;
    key_id: 8;
    tbl_id: 8;
    cq_dma_cmd_index: 8;
    inv_r_key: 32;
    dma_cmdeop: 1;
    rsvd1: 7;
    //tightly packed for 160 bits
};

struct resp_rx_key_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_rx_key_info_t args;
    struct phv_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct resp_rx_rqcb0_write_back_info_t {
    in_progress: 8;
    incr_nxt_to_go_token_id: 1;
    incr_c_index: 1;
    tbl_id: 3;
    cache: 1;
    do_not_invalidate_tbl: 1;
    pad: 145;
};

struct resp_rx_rqcb0_write_back_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_rx_rqcb0_write_back_info_t args;
    struct phv_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct resp_rx_rqcb1_write_back_info_t {
    curr_wqe_ptr: 64;
    current_sge_offset: 32;
    current_sge_id: 8;
    update_num_sges: 1;
    update_wqe_ptr: 1;
    num_sges: 8;
    pad: 46;
};

struct resp_rx_rqcb1_write_back_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_rx_rqcb1_write_back_info_t args;
    struct phv_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct resp_rx_lkey_to_pt_info_t {
    pt_offset: 32;
    pt_bytes: 16;
    dma_cmd_start_index: 8;
    sge_index: 8;
    log_page_size: 5;
    dma_cmdeop: 1;
    rsvd: 2;
    pad: 88;
};

struct resp_rx_ptseg_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_rx_lkey_to_pt_info_t args;
    struct phv_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct resp_rx_compl_or_inv_rkey_info_t {
    r_key:  32;
    dma_cmd_index: 8;
    tbl_id: 3;
    rsvd: 5;
    pad: 112;
};

struct resp_rx_compl_or_inv_rkey_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_rx_compl_or_inv_rkey_info_t args;
    struct phv_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct resp_rx_rqcb_to_cq_info_t {
    tbl_id: 3;
    rsvd: 5;
    dma_cmd_index: 8;
    pad: 144;
};

struct resp_rx_cqcb_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_rx_rqcb_to_cq_info_t args;
    struct phv_to_stage_t to_stage;
    struct phv_global_common_t global;
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
    pad: 72;
};

struct resp_rx_cqpt_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_rx_cqcb_to_pt_info_t args;
    struct phv_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct resp_rx_cqcb_to_eq_info_t {
    tbl_id: 3;
    rsvd: 5;
    dma_cmd_index: 8;  
    cq_id: 24;
    pad: 120;
};

struct resp_rx_eqcb_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_rx_cqcb_to_eq_info_t args;
    struct phv_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct resp_rx_rqcb_to_rqcb1_info_t {
    in_progress: 1;
    rsvd: 7;
    remaining_payload_bytes: 32;
    inv_r_key: 32;
    pad: 88;
};

struct resp_rx_rqcb1_in_progress_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_rx_rqcb_to_rqcb1_info_t args;
    struct phv_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct resp_rx_rqcb_to_write_rkey_info_t {
    va: 64;
    len: 32;
    r_key: 32;
    remaining_payload_bytes: 16;
    load_reth: 1;
    incr_c_index: 1;
    rsvd1: 6;
    rsvd2: 8;
};

struct resp_rx_write_dummy_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_rx_rqcb_to_write_rkey_info_t args;
    struct phv_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct resp_rx_inv_rkey_info_t {
    tbl_id: 8;
    key_id: 8;
    pad: 144;
};

struct resp_rx_inv_rkey_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_rx_inv_rkey_info_t args;
    struct phv_to_stage_t to_stage;
    struct phv_global_common_t global;
};

#endif //__RESP_RX_H
