#ifndef __RESP_RX_ARGS_H
#define __RESP_RX_ARGS_H

#include "capri.h"
#include "common_phv.h"

struct resp_rx_to_stage_backtrack_info_t {
    va: 64;
    r_key: 32;
    len: 32;
};

struct resp_rx_to_stage_wb0_info_t {
    my_token_id: 8;
    bytes: 16;
    pad: 104;
};

struct resp_rx_to_stage_wb1_info_t {
    update_wqe_ptr: 1;
    update_num_sges: 1;
    rsvd: 6;
    num_sges: 8;
    inv_r_key: 32;
    cqcb_base_addr_page_id  : 22;
    log_num_cq_entries      : 4;
    pad: 54;
};

//struct resp_rx_to_stage_cqpt_info_t {
//    cqcb_base_addr_page_id  : 22;
//    log_num_cq_entries      : 4;
//};
//
//struct resp_rx_to_stage_stats_info_t {
//    bytes: 16;
//};

struct resp_rx_to_stage_cqpt_stats_info_t {
    cqcb_base_addr_page_id  : 22;
    log_num_cq_entries      : 4;
    bytes: 16;
    pad: 86;
};

struct resp_rx_to_stage_recirc_info_t {
    curr_wqe_ptr: 64;
    current_sge_id: 6;
    num_sges: 6;
    dma_cmd_index: 4;
    remaining_payload_bytes: 16;
    current_sge_offset: 32;
};

struct resp_rx_to_stage_atomic_info_t {
    rsqwqe_ptr: 64;
    pad: 64; 
};

//Right now, worst case RDMA headers are overflowing by 64bits at maximum
//from stage0 app_data table to rdma_params_table.
//That data is passed blindly by rdma_params_table in stage0 to stage2 of
//regular program
struct resp_rx_to_stage_ext_hdr_info_t {
    ext_hdr_data: 96;
    pad: 32;
};

struct resp_rx_s0_info_t {
    union {
        struct resp_rx_to_stage_backtrack_info_t backtrack;
    };
};

struct resp_rx_s1_info_t {
    union {
        struct resp_rx_to_stage_backtrack_info_t backtrack;
        struct resp_rx_to_stage_recirc_info_t recirc;
        struct resp_rx_to_stage_atomic_info_t atomic;
    };
};

struct resp_rx_s2_info_t {
    union {
        struct resp_rx_to_stage_backtrack_info_t backtrack;
        struct resp_rx_to_stage_ext_hdr_info_t ext_hdr;
    };
};

struct resp_rx_s3_info_t {
    union {
        struct resp_rx_to_stage_backtrack_info_t backtrack;
        struct resp_rx_to_stage_wb0_info_t wb0;
    };
};

struct resp_rx_s4_info_t {
    union {
        struct resp_rx_to_stage_backtrack_info_t backtrack;
        struct resp_rx_to_stage_wb1_info_t wb1;
    };
};

struct resp_rx_s5_info_t {
    union {
        struct resp_rx_to_stage_backtrack_info_t backtrack;
    };
};

struct resp_rx_s6_info_t {
    union {
        struct resp_rx_to_stage_backtrack_info_t  backtrack;
        struct resp_rx_to_stage_cqpt_stats_info_t cqpt_stats;
    };
};

struct resp_rx_s7_info_t {
    union {
        struct resp_rx_to_stage_backtrack_info_t backtrack;
    };
};

struct resp_rx_to_stage_t {
    union {
        struct resp_rx_s0_info_t s0;
        struct resp_rx_s1_info_t s1;
        struct resp_rx_s2_info_t s2;
        struct resp_rx_s3_info_t s3;
        struct resp_rx_s4_info_t s4;
        struct resp_rx_s5_info_t s5;
        struct resp_rx_s6_info_t s6;
        struct resp_rx_s7_info_t s7;
    };
};

#endif //__RESP_RX_ARGS_H
