#ifndef __SQCB_H
#define __SQCB_H
#include "capri.h"

#define MAX_SQ_RINGS            3
#define MAX_SQ_HOST_RINGS       1

#define SQ_RING_ID              0
#define TIMER_RING_ID           1
#define RDMA_CQ_PROXY_RING_ID   2

#define TIMER_PRI               0
#define SQ_PRI                  1
#define RDMA_CQ_PROXY_PRI       2

#define SQ_RING_ID_BITMAP            0x01 // (1 << SQ_RING_ID)
#define TIMER_RING_ID_BITMAP         0x02 // (1 << TIMER_RING_ID)
#define RDMA_CQ_PROXY_RING_ID_BITMAP 0x03 // (1 << RDMA_CQ_PROXY_RING_ID)

#define SQ_P_INDEX                   d.{ring0.pindex}.hx
#define SQ_C_INDEX                   d.{ring0.cindex}.hx
#define SQ_TIMER_P_INDEX             d.{ring1.pindex}.hx
#define SQ_TIMER_C_INDEX             d.{ring1.cindex}.hx
#define RDMA_CQ_PROXY_P_INDEX        d.{ring2.pindex}.hx
#define RDMA_CQ_PROXY_C_INDEX        d.{ring2.cindex}.hx

#define SQCB_T struct sqcb_t
#define SQCB0_T struct sqcb0_t
#define SQCB1_T struct sqcb1_t
#define SQCB2_T struct sqcb2_t

#define SQCB_SQ_PINDEX_OFFSET            FIELD_OFFSET(sqcb0_t, ring0.pindex)
#define SQCB_SQ_CINDEX_OFFSET            FIELD_OFFSET(sqcb0_t, ring0.cindex)
#define SQCB_RDMA_CQ_PROXY_PINDEX_OFFSET FIELD_OFFSET(sqcb0_t, ring2.pindex)
#define SQCB_RDMA_CQ_PROXY_CINDEX_OFFSET FIELD_OFFSET(sqcb0_t, ring2.cindex)

#define SQ_WQE_CONTEXT_TYPE_NONE 0
#define SQ_WQE_CONTEXT_TYPE_MR   1
#define SQ_WQE_CONTEXT_TYPE_SEND 2

#define LOG_WQE_CONTEXT_SIZE 8 //256 bits

struct sq_wqe_context_t {
    type: 8;
    pad : 88;
    wrid : 64;
    struct {
        //QP local mr_id
        mr_id1: 12;
        mr_id2: 12;
        mr_id3: 12;
        mr_id4: 12;
    } mr;
    //RDMA MSN values assocated with this req
    start_msn: 24;
    end_msn  : 24;
};

struct sq_wqe_context_aligned_t {
    struct sq_wqe_context_t ctx;
    pad: 256;
};

struct sqcb0_t {
    struct capri_intrinsic_qstate_t intrinsic;
    struct capri_intrinsic_ring_t ring0;
    struct capri_intrinsic_ring_t ring1;
    struct capri_intrinsic_ring_t ring2;
    pad                           : 32;

    //smbcd sq
    //assumption: this queue is in host contiguous memory
    sq_base_addr                  : 64; // RO
    sq_unack_pindex               : 16;
    sq_wqe_context_base_addr      : 34;

    //smbdc state
    max_fragmented_size           : 32;
    max_send_size                 : 32;
    current_sge_id                : 8;
    current_sge_offset            : 32;
    current_total_offset          : 32;
    send_in_progress              : 1;
    busy                          : 1;

    //rdma proxy cq
    rdma_cq_processing_in_prog    : 1;
    rdma_cq_processing_busy       : 1;
    rdma_cq_log_num_wqes          : 5;

    //smbdc cq
    cqcb_addr                     : 34;

    log_pmtu                      : 5;  // RO
    log_sq_page_size              : 5;  // RO
    log_wqe_size                  : 5;  // RO
    log_num_wqes                  : 5;  // RO
    sq_in_hbm                     : 1;  // RO
    ring_empty_sched_eval_done    : 1;  // RW S0
    pad2                          : 5;
};

#define NUM_ROWS                      7
#define MRS_PER_ROW                   64
#define MAX_MRS_PER_SMBDC_CONNECTION (MRS_PER_ROW * NUM_ROWS) //448

struct sqcb1_t {
    mr_base                       : 24;
    total_use_count               : 9;
    pad                           : 31;
    row0                          : 64;
    row1                          : 64;
    row2                          : 64;
    row3                          : 64;
    row4                          : 64;
    row5                          : 64;
    row6                          : 64;
};

#define RDMA_SQ_WQE_CONTEXT_LOG_SIZE 8

struct rdma_sq_wqe_local_context_t {
    pad                           : 64;
};

//196 bits
struct rdma_smbdc_header_t {
    credits_requested             : 16;
    credits_granted               : 16;
    flags                         : 16;
    reserved                      : 16;
    remaining_data_length         : 32;
    data_offset                   : 32;
    data_length                   : 32;
    padding                       : 32;
    //buffer[];
};

struct rdma_sq_wqe_context_t {
    struct rdma_sq_wqe_local_context_t local;
    struct rdma_smbdc_header_t smbdc_hdr;
};

struct sqcb2_t {
    rdma_sq_base_addr             : 64;
    rdma_sq_pindex                : 16;
    rdma_sq_cindex                : 16;
    rdma_sq_log_num_wqes          : 6;
    rdma_sq_log_wqe_size          : 5;
    rdma_sq_curr_msn              : 24;
    rdma_lif                      : 12;
    rdma_qtype                    : 3;
    rdma_qid                      : 24;
    rdma_sq_msn                   : 24;

    rdma_local_dma_lkey           : 32;

    //local-context + SMBDC header associated with RDMA WQE
    rdma_sq_wqe_context_base_addr : 34;

    //smbdc protocol parameters
    //set by control plane, and used by req_tx
    //never chagned by data plane
    credits_requested             : 16;
    //set by resp_rx, used and reset by req_tx
    //in s3_t0
    credits_granted               : 16;
    pad                           : 220;
};

struct sqcb3_t {
    //rdma cq
    //assumption: HBM resident
    rdma_cq_base_addr             : 34;
    rdma_cq_log_wqe_size          : 5;
    rdma_cq_log_num_wqes          : 5;
    rdma_cq_msn                   : 24;
    rdma_cq_rsvd_flags            : 6;
    rdma_cq_lif                   : 12;
    rdma_cq_qtype                 : 3;
    rdma_cq_qid                   : 24;
    rdma_cq_ring_id               : 3;

    pad                           : 396;
};

struct sqcb_t {
    struct sqcb0_t sqcb0;
    struct sqcb1_t sqcb1;
    struct sqcb2_t sqcb2;
    struct sqcb3_t sqcb3;
};

#endif //__SQCB_H
