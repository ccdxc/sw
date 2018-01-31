#ifndef __REQ_TX_ARGS_H
#define __REQ_TX_ARGS_H

#include "capri.h"
#include "common_phv.h"

struct req_tx_sqcb_to_pt_info_t {
    page_offset                    : 16;
    page_seg_offset                : 3;
    remaining_payload_bytes        : 32;
    rrq_p_index                    : 8;
    //packed params begin - pd, log_pmtu
    pd                             : 32;
    log_pmtu                       : 5;
    //packed params end
    pad                            : 64; // pad to 160bits for S2S data
};

struct req_tx_sqcb_to_wqe_info_t {
    in_progress                    : 1;
    log_pmtu                       : 5;
    li_fence_cleared               : 1;
    current_sge_id                 : 8;
    num_valid_sges                 : 8;
    current_sge_offset             : 32;
    remaining_payload_bytes        : 16;
    rrq_p_index                    : 8;
    pd                             : 32;
    pad                            : 49;
};

struct req_tx_wqe_to_sge_info_t {
    in_progress                    : 1;
    op_type                        : 8;
    first                          : 1;
    current_sge_id                 : 8;
    num_valid_sges                 : 8;
    current_sge_offset             : 32;
    remaining_payload_bytes        : 16;
    ah_size                        : 8;
    rsvd                           : 8;
    dma_cmd_start_index            : 6; // TODO Different from "C" code due to space scarcity
    imm_data                       : 32;
    union {
        inv_key                        : 32;
        ah_handle                      : 32;
    };
};

struct req_tx_sge_to_lkey_info_t {
    sge_va                        : 64;
    sge_bytes                     : 16;
    dma_cmd_start_index           : 8;
    sge_index                     : 8;
    pad                           : 64; 
};

struct req_tx_lkey_to_ptseg_info_t {
    pt_offset                    : 32;
    log_page_size                : 5;
    pt_bytes                     : 16;
    dma_cmd_start_index          : 8;
    sge_index                    : 8;
    pad                          : 91;
};

struct rd_t {
    read_len                      : 32;
//    num_sges                      : 8;
    log_pmtu                      : 5;
//    pad                           : 19;
    pad                           : 27;
};

struct send_wr_t {
    imm_data                      : 32;
    union {
        inv_key                       : 32;
        ah_handle                     : 32;
    };
};

union op_t {
    struct rd_t rd;
    struct send_wr_t send_wr;
};

struct req_tx_rrqwqe_to_hdr_info_t {
    rsvd                          : 2;
    last                          : 1;
    //keep op_type...first contiguous
    op_type                       : 4;
    first                         : 1;
    tbl_id                        : 3;
    log_pmtu                      : 5;
    rrq_p_index                   : 8;
    union op_t op;
};

struct req_tx_sqcb_write_back_info_t {
    busy                         : 1;
    in_progress                  : 1;
    //keep op_type...first contiguous
    op_type                      : 8;
    first                        : 1;
    last                         : 1;
    set_fence                    : 1;
    set_li_fence                 : 1;
    empty_rrq_bktrack            : 1;
    release_cb1_busy             : 1;
    num_sges                     : 8;
    current_sge_id               : 8;
    current_sge_offset           : 32;
    sq_c_index                   : 16;
    union {
        rrq_p_index                  : 16;
        struct {
            ah_size                  : 8;
            rsvd                     : 8;
        };
    };
    union op_t op;
};

// Note: Do not change the order of log_pmtu to num_sges as
// program uses concatenated copy like k.{log_pmtu...num_sges}
// from  req_tx_sqcb0_to_sqcb1_info to req_tx_sqcb1_to_bktrack_wqe_info
struct req_tx_sqcb0_to_sqcb1_info_t {
    sq_c_index                    : 16;
    sq_p_index                    : 16;
    in_progress                   : 1;
    bktrack_in_progress           : 1;
    current_sge_offset            : 32;
    current_sge_id                : 8;
    num_sges                      : 8;
    update_credits                : 1;
    bktrack                       : 1; 
    pt_base_addr                  : 32;
    op_type                       : 5;
    sq_in_hbm                     : 1;
    pad                           : 38;
};


struct req_tx_sqcb1_to_credits_info_t {
    need_credits                 : 1;
    pad                          : 159;
};

// Note: Do not change the order of sq_c_index to num_sges as
// program uses concatenated copy like k.{log_pmtu...num_sges}
// from  req_tx_sqcb0_to_sqcb1_info to req_tx_sq_bktrack_info
//
struct req_tx_sq_bktrack_info_t {
    sq_c_index                    : 16;
    in_progress                   : 1;
//    bktrack_in_progress           : 1;
    current_sge_offset            : 32;
    current_sge_id                : 8;
    num_sges                      : 8;
    tx_psn                        : 24;
    ssn                           : 24;
    op_type                       : 5;
    union {
        struct {
            sq_p_index            : 16;
            pad1                  : 26;
        };
        struct {
            union {
                imm_data          : 32;       
                inv_key           : 32;
            };
            pad2                  : 10;       
        };
    };
};

struct req_tx_bktrack_sqcb1_write_back_info_t {
    wqe_start_psn                 : 24;
    tx_psn                        : 24;
    ssn                           : 24;
    skip_wqe_start_psn            : 1;
    tbl_id                        : 3;
    imm_data                      : 32;
    inv_key                       : 32;
    pad                           : 20;
};

struct req_tx_sq_to_stage_t {
    wqe_addr                     : 64;
    spec_cindex                  : 16; 
    header_template_addr         : 32;
    packet_len                   : 14;
    congestion_mgmt_enable       : 1;   
    rate_enforce_failed          : 1;
};

struct req_tx_bktrack_to_stage_t {
    wqe_addr                     : 64;
    //packed params begin: log_pmtu, log_sq_page_size, log_wqe_size, log_num_wqes
    log_pmtu                     : 5;
    log_sq_page_size             : 5;
    log_wqe_size                 : 5;
    log_num_wqes                 : 5;
    //packed params end: log_pmtu, log_sq_page_size, log_wqe_size, log_num_wqes
    rexmit_psn                   : 24;
    pad                          : 20;
};

struct req_tx_to_stage_t {
    union {
        struct req_tx_sq_to_stage_t sq;
        struct req_tx_bktrack_to_stage_t bktrack;
    };
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
