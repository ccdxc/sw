#ifndef __REQ_TX_H
#define __REQ_TX_H
#include "capri.h"
#include "types.h"
#include "req_tx_args.h"
#include "INGRESS_p.h"
#include "common_phv.h"

#define REQ_TX_RDMA_PAYLOAD_DMA_CMDS_START 8
#define REQ_TX_DMA_CMD_START_FLIT_ID 8
#define TOTAL_DMA_CMD_BITS 16 * 16 * 8 // (cmds * dma_cmd_size * bits_per_byte) 

#define BTH_DST_QP              p.bth.dst_qp
#define BTH_PSN                 p.bth.psn
#define BTH_ACK_REQ             p.bth.a
#define BTH_OPCODE              p.bth.opcode

#define IMMDT_DATA              p.immeth.data

#define IETH_R_KEY              p.ieth.rkey

#define RETH_VA                 p.reth.va
#define RETH_RKEY               p.reth.r_key
#define RETH_LEN                p.reth.dma_len

#define ATOMIC_VA               p.atomiceth.va
#define ATOMIC_R_KEY            p.atomiceth.r_key
#define ATOMIC_SWAP_OR_ADD_DATA p.atomiceth.swap_or_add_data
#define ATOMIC_CMP_DATA         p.atomiceth.cmp_data

#define RRQWQE_READ_RSP_OR_ATOMIC         p.rrqwqe.read_rsp_or_atomic
#define RRQWQE_NUM_SGES                   p.rrqwqe.num_sges
#define RRQWQE_PSN                        p.rrqwqe.psn
#define RRQWQE_MSN                        p.rrqwqe.msn
#define RRQWQE_READ_LEN                   p.rrqwqe.read.len
#define RRQWQE_READ_WQE_SGE_LIST_ADDR     p.rrqwqe.read.wqe_sge_list_addr
#define RRQWQE_ATOMIC_SGE_VA              p.rrqwqe.atomic.sge.va
#define RRQWQE_ATOMIC_SGE_LEN             p.rrqwqe.atomic.sge.len
#define RRQWQE_ATOMIC_SGE_LKEY            p.rrqwqe.atomic.sge.l_key

// phv 
struct req_tx_phv_t {
    // dma commands

    /* flit 11 */
    dma_cmd12 : 128;
    dma_cmd13 : 128;
    dma_cmd14 : 128;
    dma_cmd15 : 128;

    /* flit 10 */
    dma_cmd8 : 128;
    dma_cmd9 : 128;
    dma_cmd10 : 128;
    dma_cmd11 : 128;

    /* flit 9 */
    dma_cmd4 : 128;
    dma_cmd5 : 128;
    dma_cmd6 : 128;
    dma_cmd7 : 128;
 
    /* flit 8 */
    dma_cmd0 : 128;
    dma_cmd1 : 128;
    dma_cmd2 : 128;
    dma_cmd3 : 128;

    /* flit 7 */
    rsvd_flit_7 : 120;
    rrq_p_index: 16;
    union {
        struct rrqwqe_t rrqwqe;
        inline_data: 256;
    };
    struct p4plus_to_p4_header_t p4plus_to_p4;
    rsvd       : 6;
    in_progress: 1;
    rrq_empty: 1;

    /* flit 6 */
    struct rdma_ieth_t ieth;
    struct rdma_atomiceth_t atomiceth;
    struct rdma_immeth_t immeth;
    struct rdma_reth_t reth;
    struct rdma_bth_t bth;  // should be from a byte boundary

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
    struct req_tx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct req_tx_sqpt_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct req_tx_sqcb_to_pt_info_t args;
    struct req_tx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct req_tx_sqwqe_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct req_tx_sqcb_to_wqe_info_t args;
    struct req_tx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct req_tx_sqsge_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct req_tx_wqe_to_sge_info_t args;
    struct req_tx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct req_tx_sqlkey_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct req_tx_sge_to_lkey_info_t args;
    struct req_tx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct req_tx_sqptseg_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct req_tx_lkey_to_ptseg_info_t args;
    struct req_tx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct req_tx_add_headers_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct req_tx_rrqwqe_to_hdr_info_t args;
    struct req_tx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct req_tx_write_back_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct req_tx_sqcb_write_back_info_t args;
    struct req_tx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct req_tx_sqcb1_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct req_tx_sqcb0_to_sqcb1_info_t args;
    struct req_tx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct req_tx_credits_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct req_tx_sqcb1_to_credits_info_t args;
    struct req_tx_to_stage_t to_stage;
    struct phv_global_common_t global;
};


#endif //__REQ_TX_H
