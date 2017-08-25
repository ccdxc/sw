#ifndef __REQ_TX_H
#define __REQ_TX_H
#include "capri.h"
#include "req_tx_args.h"
#include "txdma_phv.h"
#include "common_phv.h"

#define REQ_TX_RDMA_PAYLOAD_DMA_CMDS_START 8

// phv 
struct req_tx_phv_t {
    // dma commands

    // scratch
    in_progress: 1;
    rrq_empty: 1;

    // common tx
    struct phv_ common;
};

struct req_tx_phv_global_t {
    struct phv_global_common_t common;
};

// stage to stage argument structures

struct req_tx_s0_t {
    lif: 11;
    qid: 24;
    qtype: 3;
};

struct req_tx_s0_k_t {
    struct capri_intrinsic_s0_k_t intrinsic;
    struct req_tx_s0_t args;
};

struct req_tx_sqpt_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct phv_global_common_t global;
    struct req_tx_sqcb_to_pt_info_t args;
};

struct req_tx_sqwqe_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct phv_global_common_t global;
    struct req_tx_sqcb_to_wqe_info_t args;
};

struct req_tx_sqsge_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct phv_global_common_t global;
    struct req_tx_wqe_to_sge_info_t args;
};

struct req_tx_sqlkey_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct phv_global_common_t global;
    struct req_tx_sge_to_lkey_info_t args;
};

struct req_tx_sqptseg_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct phv_global_common_t global;
    struct req_tx_lkey_to_ptseg_info_t args;
};

#endif //__REQ_TX_H
