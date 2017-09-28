#ifndef __RESP_TX_H
#define __RESP_TX_H
#include "capri.h"
#include "types.h"
#include "resp_tx_args.h"
#include "INGRESS_p.h"
#include "common_phv.h"
#include "defines.h"

#define RESP_TX_MAX_DMA_CMDS            16
#define RESP_TX_DMA_CMD_INTRINSIC       0
#define RESP_TX_DMA_CMD_COMMON_P4PLUS   1
#define RESP_TX_DMA_CMD_HDR_TEMPLATE    2
#define RESP_TX_DMA_CMD_BTH             3
#define RESP_TX_DMA_CMD_AETH            4
#define RESP_TX_DMA_CMD_PYLD_BASE       5

#define RESP_TX_DMA_CMD_START_FLIT_ID   8 // flits 8-11 are used for dma cmds

// phv 
struct resp_tx_phv_t {
    // dma commands (flit 8 - 11)

    // scratch (flit 6 - 7)
    db_data: 32;
    struct rdma_aeth_t aeth;
    struct rdma_bth_t bth;
    struct p4plus_to_p4_header_t p4plus_to_p4;

    // common tx (flit 0 - 5)
    struct phv_ common;
};

struct resp_tx_phv_global_t {
    struct phv_global_common_t common;
};

// stage to stage argument structures

struct resp_tx_rqcb_to_rqcb1_info_t {
    rsqwqe_addr: 64;
    log_pmtu: 5;
    serv_type: 3;
    new_rsq_c_index: 16;
};

struct resp_tx_rqcb1_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_tx_rqcb_to_rqcb1_info_t args;
    struct resp_tx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct resp_tx_rqcb1_to_rsqwqe_info_t {
    curr_read_rsp_psn: 24;
    log_pmtu: 5;
    serv_type: 3;
    header_template_addr: 32;
    read_rsp_in_progress: 1;
    rsvd0: 7;
    new_rsq_c_index: 16;
};

struct resp_tx_rsqwqe_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_tx_rqcb1_to_rsqwqe_info_t args;
    struct resp_tx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct resp_tx_rsqwqe_to_rkey_info_t {
    transfer_va: 64;
    header_template_addr: 32;
    curr_read_rsp_psn: 24;
    log_pmtu: 5;
    key_id: 1;
    send_aeth: 1;
    last_or_only: 1;
    transfer_bytes: 16;
    new_rsq_c_index: 16;
};

struct resp_tx_rsqrkey_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_tx_rsqwqe_to_rkey_info_t args;
    struct resp_tx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct resp_tx_rqcb1_write_back_info_t {
    curr_read_rsp_psn: 24;
    read_rsp_in_progress: 1;
    rsvd0: 7;
    new_rsq_c_index: 16;
};

struct resp_tx_rqcb1_write_back_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_tx_rqcb1_write_back_info_t args;
    struct resp_tx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct resp_tx_rkey_to_ptseg_info_t {
    pt_seg_offset: 32;
    pt_seg_bytes: 32;
    dma_cmd_start_index: 8;
    log_page_size: 5;
    tbl_id: 2;
    dma_cmdeop: 1;
};

struct resp_tx_rsqptseg_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_tx_rkey_to_ptseg_info_t args;
    struct resp_tx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct resp_tx_rqcb_to_ack_info_t {
    //dst_qp: 24;
    serv_type: 8;
    new_c_index: 16;
};

struct resp_tx_ack_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_tx_rqcb_to_ack_info_t args;
    struct resp_tx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

#endif //__RESP_TX_H
