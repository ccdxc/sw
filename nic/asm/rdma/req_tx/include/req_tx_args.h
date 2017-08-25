#ifndef __REQ_TX_ARGS_H
#define __REQ_TX_ARGS_H

#include "capri.h"
#include "common_phv.h"

struct req_tx_sqcb_to_pt_info_t {
    page_offset                    : 16;
    page_seg_offset                : 3;
    remaining_payload_bytes        : 32;
    rrq_p_index                    : 8;
    log_pmtu                       : 5;
    pd                             : 32;
};

struct req_tx_sqcb_to_wqe_info_t {
    in_progress                    : 1;
    log_pmtu                       : 5;
    li_fence_cleared               : 1;
    current_sge_id                 : 8;
    num_valid_sges                 : 8;
    current_sge_offset             : 32;
    remaining_payload_bytes        : 16;
    wqe_addr                       : 64;
    rrq_p_index                    : 8;
    pd                             : 32;
};

struct req_tx_wqe_to_sge_info_t {
    in_progress                    : 1;
    first                          : 1;
    current_sge_id                 : 8;
    num_valid_sges                 : 8;
    current_sge_offset             : 8;
    remaining_payload_bytes        : 16;
    payload_offset                 : 16;
    dma_cmd_start_index            : 8;
    op_type                        : 8;
    wqe_addr                       : 64;
    imm_data                       : 32;
    inv_key                        : 32;
};

struct req_tx_sge_to_lkey_info_t {
    sge_va                        : 64;
    sge_bytes                     : 16;
    log_page_size                 : 5;
    dma_cmd_start_index           : 16;
    key_id                        : 8;
    sge_index                     : 8;
};

struct rd_t {
    read_len                      : 32;
    wqe_sge_list_addr             : 64;
    num_sges                      : 8;
};

struct send_wr_t {
    current_sge_offset            : 32;
    current_sge_id                : 8;
    num_sges                      : 8;
    imm_data                      : 32;
    inv_key                       : 32;
};

union op_t {
    struct rd_t rd;
    struct send_wr_t send_wr;
};

struct req_tx_rrqwqe_to_hdr_info_t {
    busy                          : 1;
    in_progress                   : 1;
    first                         : 1;
    last                          : 1;
    op_type                       : 4;
    tbl_id                        : 3;
    log_pmtu                      : 5;
    rrq_p_index                   : 8;
    wqe_addr                      : 64;
    union op_t op;
};

struct req_tx_lkey_to_ptseg_info_t {
    pt_offset                    : 32;
    pt_bytes                     : 16;
    log_page_size                : 16;
    dma_cmd_start_index          : 8;
};

/*
struct req_tx_sqcb_to_wqe_info_k_t {
    struct capri_intrinsic_k_t intrinsic;
    union {
        global_data: GLOBAL_DATA_WIDTH;
        struct req_tx_phv_global_t global;
    };
    union {
       s2s_data: S2S_DATA_WIDTH;
       struct req_tx_sqcb_to_wqe_info_t params;
    };
};
*/

union args_union_t {
    //struct req_tx_sqcb_to_wqe_info_t sqcb_to_wqe;
    dummy: 32;
};

#endif //__REQ_TX_ARGS_H
