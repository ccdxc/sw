#ifndef __REQ_RX_H
#define __REQ_RX_H
#include "capri.h"
#include "types.h"
#include "req_rx_args.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "common_phv.h"

#define REQ_RX_CQCB_ADDR_GET(_r, _cqid) \
    CQCB_ADDR_GET(_r, _cqid, k.to_stage.cqcb_base_addr_page_id);

#define REQ_RX_EQCB_ADDR_GET(_r, _tmp_r, _eqid) \
    EQCB_ADDR_GET(_r, _tmp_r, _eqid, k.to_stage.cqcb_base_addr_page_id, k.to_stage.log_num_cq_entries);

#define REQ_RX_DMA_CMD_START_FLIT_ID       8 // flits 8-11 are used for dma cmds
#define REQ_RX_DMA_CMD_START               0
#define REQ_RX_BKTRACK_PSN_DMA_CMD         3
#define REQ_RX_RDMA_PAYLOAD_DMA_CMDS_START 4
#define REQ_RX_MAX_DMA_CMDS                16
#define REQ_RX_DMA_CMD_CQ                  (REQ_RX_MAX_DMA_CMDS - 3)

// phv 
struct req_rx_phv_t {
    // dma commands

    // scratch
    rexmit_psn              : 24;
    msn                     : 24;
    rsvd                    : 3;
    credits                 : 5;
    db_data2                : 64;
    db_data1                : 64;
    eq_int_num              : 16;
    struct eqwqe_t eqwqe;
    struct cqwqe_t cqwqe;
    my_token_id             : 8;
    // common rx
    struct phv_ common;
};

struct req_rx_phv_global_t {
    struct phv_global_common_t common;
};

// stage to stage argument structures

struct req_rx_s0_t {
    lif: 11;
    qtype: 3;
    qid: 24;
    struct p4_2_p4plus_app_hdr_t app_hdr;
};

struct req_rx_s0_k_t {
    struct capri_intrinsic_s0_k_t intrinsic;
    struct req_rx_s0_t args;
    struct req_rx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct req_rx_sqcb1_process_k_t {
    struct capri_intrinsic_s0_k_t intrinsic;
    struct req_rx_sqcb0_to_sqcb1_info_t args;
    struct req_rx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct req_rx_rrqwqe_process_k_t {
    struct capri_intrinsic_s0_k_t intrinsic;
    struct req_rx_sqcb1_to_rrqwqe_info_t args;
    struct req_rx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct req_rx_rrqsge_process_k_t {
    struct capri_intrinsic_s0_k_t intrinsic;
    struct req_rx_rrqwqe_to_sge_info_t args;
    struct req_rx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct req_rx_rrqlkey_process_k_t {
    struct capri_intrinsic_s0_k_t intrinsic;
    struct req_rx_rrqsge_to_lkey_info_t args;
    struct req_rx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct req_rx_rrqptseg_process_k_t {
    struct capri_intrinsic_s0_k_t intrinsic;
    struct req_rx_rrqlkey_to_ptseg_info_t args;
    struct req_rx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct req_rx_sqcb1_write_back_process_k_t {
    struct capri_intrinsic_s0_k_t intrinsic;
    struct req_rx_sqcb1_write_back_info_t args;
    struct req_rx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct req_rx_cqcb_process_k_t {
    struct capri_intrinsic_s0_k_t intrinsic;
    struct req_rx_rrqwqe_to_cq_info_t args;
    struct req_rx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct req_rx_cqpt_process_k_t {
    struct capri_intrinsic_s0_k_t intrinsic;
    struct req_rx_cqcb_to_pt_info_t args;
    struct req_rx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct req_rx_eqcb_process_k_t {
    struct capri_intrinsic_s0_k_t intrinsic;
    struct req_rx_cqcb_to_eq_info_t args;
    struct req_rx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct req_rx_ecn_process_k_t {
    struct capri_intrinsic_s0_k_t intrinsic;
    struct req_rx_ecn_info_t args;
    struct req_rx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

#endif //__REQ_RX_H
