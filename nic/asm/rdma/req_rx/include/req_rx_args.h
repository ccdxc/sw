#ifndef __REQ_RX_ARGS_H
#define __REQ_RX_ARGS_H

#include "capri.h"
#include "common_phv.h"

struct req_rx_to_stage_t {
    msn                     : 24;
    bth_psn                 : 24; 
    syndrome                :  8;
    cqcb_base_addr_page_id  : 22;
    log_num_cq_entries      : 4;
    pad                     : 46;
};

struct req_rx_sqcb0_to_sqcb1_info_t {
    remaining_payload_bytes : 32;
    rrq_cindex              : 8;
    rrq_empty               : 1;
    need_credits            : 1;
    dma_cmd_start_index     : 4;
    ecn_set                 : 1;
    p_key                   : 16;
    pad                     : 97;
};

struct req_rx_sqcb1_to_rrqwqe_info_t {
    cur_sge_offset          : 32;
    remaining_payload_bytes : 32;
    cq_id                   : 24;
    cur_sge_id              : 8;
    e_rsp_psn               : 24;
    msn                     : 24;
    in_progress             : 1;
    rrq_empty               : 1;
    timer_active            : 1;
    dma_cmd_start_index     : 4;
    rrq_cindex              : 8;
};

struct req_rx_rrqwqe_to_sge_info_t {
    remaining_payload_bytes : 32;
    cur_sge_offset          : 32;
    //wqe_addr                : 32;
    cq_id                   : 24;
    cur_sge_id              : 8;
    e_rsp_psn               : 24;
    num_valid_sges          : 8;
    in_progress             : 1;
    is_atomic               : 1;
    dma_cmd_eop             : 1;
    dma_cmd_start_index     : 4;
    rrq_cindex              : 8;
    pad                     : 17;
};

struct req_rx_rrqsge_to_lkey_info_t {
    sge_va                  : 64;
    sge_bytes               : 16;
    log_page_size           : 5;
    is_atomic               : 1;
    dma_cmd_eop             : 1;
    dma_cmd_start_index     : 8;
    sge_index               : 8;
    cq_dma_cmd_index        : 8;
    cq_id                   : 24;
    pad                     : 25;
};

struct req_rx_sqcb1_write_back_info_t {
    cur_sge_offset          : 32;
    e_rsp_psn               : 24;
    cur_sge_id              : 8;
    in_progress             : 1;
    incr_nxt_to_go_token_id : 1;
    post_bktrack            : 1;
    dma_cmd_eop             : 1;
    num_sges                : 8;
    tbl_id                  : 8;
    pad                     : 66;
};

struct req_rx_rrqlkey_to_ptseg_info_t {
    pt_offset               : 32;
    pt_bytes                : 16;
    log_page_size           : 5;
    dma_cmd_start_index     : 8;
    is_atomic               : 1;
    dma_cmd_eop             : 1;
    sge_index               : 8;
    pad                     : 89;
};

struct req_rx_rrqwqe_to_cq_info_t {
    tbl_id                  : 4;
    dma_cmd_index           : 8;
    pad                     : 148;
};

struct req_rx_cqcb_to_pt_info_t {
    page_offset             : 16;
    page_seg_offset         : 8;
    dma_cmd_index           : 8;
    eq_id                   : 24;
    cq_id                   : 24;
    arm                     : 1;
    wakeup_dpath            : 1;
    tbl_id                  : 3;
    rsvd                    : 4;
    pad                     : 71;
};

struct req_rx_cqcb_to_eq_info_t {
    tbl_id                  : 3;
    rsvd                    : 5;
    dma_cmd_index           : 8;
    cq_id                   : 24;
    pad                     : 120;
};

struct req_rx_ecn_info_t { 
    p_key: 16;              
    pad : 144; 
};

#endif //__REQ_RX_ARGS_H
