#ifndef __REQ_TX_H
#define __REQ_TX_H
#include "capri.h"
#include "types.h"
#include "req_tx_args.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "common_phv.h"

#define REQ_TX_DMA_CMD_PHV_INTRINSIC 2
#define REQ_TX_DMA_CMD_FRPMR_PT_TABLE_SRC_ADDR 2
#define REQ_TX_DMA_CMD_RDMA_ERR_FEEDBACK 2
#define REQ_TX_DMA_CMD_SQ_DRAIN_FEEDBACK 2
#define REQ_TX_DMA_CMD_HEADER_TEMPLATE 3
#define REQ_TX_DMA_CMD_FRPMR_PT_TABLE_DST_ADDR 3
#define REQ_TX_DMA_CMD_RDMA_HEADERS 4
#define REQ_TX_DMA_CMD_SET_FRPMR_IN_PROGRESS 4
#define REQ_TX_DMA_CMD_RRQWQE 5
#define REQ_TX_DMA_CMD_RRQWQE_BASE_SGES 6
#define REQ_TX_DMA_CMD_RRQ_PINDEX 7
#define REQ_TX_DMA_CMD_PYLD_BASE 5
#define REQ_TX_DMA_CMD_PYLD_BASE_END 17
#define REQ_TX_DMA_CMD_RDMA_PAD_ICRC 17
#define REQ_TX_DMA_CMD_RDMA_UDP_OPTS 18
#define REQ_TX_DMA_CMD_RDMA_FEEDBACK 19   // This should be at the end
#define REQ_TX_DMA_CMD_START_FLIT_ID 7
#define REQ_TX_DMA_CMD_START_FLIT_CMD_ID 2
#define TOTAL_DMA_CMD_BITS 16 * 16 * 8 // (cmds * dma_cmd_size * bits_per_byte) 

// DMA commands design for CNP 
#define REQ_TX_DMA_CMD_CNP_HEADER_TEMPLATE_1 3
#define REQ_TX_DMA_CMD_CNP_HEADER_TEMPLATE_2 4
#define REQ_TX_DMA_CMD_CNP_HEADER_TEMPLATE_3 5
#define REQ_TX_DMA_CMD_CNP_RDMA_HEADERS 6

#define BTH_DST_QP              p.bth.dst_qp
#define BTH_PSN                 p.bth.psn
#define BTH_ACK_REQ             p.bth.a
#define BTH_OPCODE              p.bth.opcode
#define BTH_PKEY                p.bth.pkey

#define IMMDT_DATA              p.immeth.data

#define IETH_R_KEY              p.ieth.rkey

#define RETH_VA                 p.reth.va
#define RETH_RKEY               p.reth.r_key
#define RETH_LEN                p.reth.dma_len

#define ATOMIC_VA               p.atomiceth.va
#define ATOMIC_R_KEY            p.atomiceth.r_key
#define ATOMIC_SWAP_OR_ADD_DATA p.atomiceth.swap_or_add_data
#define ATOMIC_CMP_DATA         p.atomiceth.cmp_data

#define DETH_Q_KEY              p.deth.q_key
#define DETH_SRC_QP             p.deth.src_qp

#define RRQWQE_READ_RSP_OR_ATOMIC         p.rrqwqe.read_rsp_or_atomic
#define RRQWQE_WQE_FORMAT                 p.rrqwqe.wqe_format
#define RRQWQE_NUM_SGES                   p.rrqwqe.num_sges
#define RRQWQE_PSN                        p.rrqwqe.psn
#define RRQWQE_MSN                        p.rrqwqe.msn
#define RRQWQE_READ_LEN                   p.rrqwqe.read.len
#define RRQWQE_READ_WQE_SGE_LIST_ADDR     p.rrqwqe.read.wqe_sge_list_addr
#define RRQWQE_READ_BASE_SGES             p.rrqwqe_base_sges
#define RRQWQE_ATOMIC_SGE_VA              p.rrqwqe.atomic.sge.va
#define RRQWQE_ATOMIC_SGE_LEN             p.rrqwqe.atomic.sge.len
#define RRQWQE_ATOMIC_SGE_LKEY            p.rrqwqe.atomic.sge.l_key
#define RRQWQE_ATOMIC_OP_TYPE             p.rrqwqe.atomic.op_type

#define REQ_RX_RECIRC_REASON_NONE                0
#define REQ_TX_RECIRC_REASON_SGE_WORK_PENDING    1
#define REQ_TX_RECIRC_REASON_BIND_MW             2

//ROCE UDP options related definitions

#define ROCE_OPT_KIND_EOL 0
#define ROCE_OPT_KIND_NOP 1
#define ROCE_OPT_KIND_OCS 2
#define ROCE_OPT_KIND_MSS 5
#define ROCE_OPT_KIND_TS  6

#define ROCE_OPT_LEN_MSS  4
#define ROCE_OPT_LEN_TS  10

// DCQCN CNP dscp/pcp value related definitions
#define IPV4_HEADER_TEMPLATE_LEN_UNTAG  42
#define IPV4_HEADER_TEMPLATE_LEN_TAG    46
#define CNP_IPV4_HEADER_TEMPLATE_1_LEN_UNTAG 15
#define CNP_IPV4_HEADER_TEMPLATE_1_LEN_TAG 19
#define CNP_IPV4_HEADER_TEMPLATE_2_LEN  1
#define CNP_IPV4_HEADER_TEMPLATE_3_LEN  26
#define CNP_IPV4_HEADER_TEMPLATE_3_ADDR_OFFSET_UNTAG CNP_IPV4_HEADER_TEMPLATE_1_LEN_UNTAG + CNP_IPV4_HEADER_TEMPLATE_2_LEN
#define CNP_IPV4_HEADER_TEMPLATE_3_ADDR_OFFSET_TAG CNP_IPV4_HEADER_TEMPLATE_1_LEN_TAG + CNP_IPV4_HEADER_TEMPLATE_2_LEN

#define IPV6_HEADER_TEMPLATE_LEN_UNTAG  62
#define IPV6_HEADER_TEMPLATE_LEN_TAG    66
#define CNP_IPV6_HEADER_TEMPLATE_1_LEN_UNTAG  14
#define CNP_IPV6_HEADER_TEMPLATE_1_LEN_TAG  18
#define CNP_IPV6_HEADER_TEMPLATE_2_LEN  2
#define CNP_IPV6_HEADER_TEMPLATE_3_LEN  46
#define CNP_IPV6_HEADER_TEMPLATE_3_ADDR_OFFSET_UNTAG CNP_IPV6_HEADER_TEMPLATE_1_LEN_UNTAG + CNP_IPV6_HEADER_TEMPLATE_2_LEN
#define CNP_IPV6_HEADER_TEMPLATE_3_ADDR_OFFSET_TAG CNP_IPV6_HEADER_TEMPLATE_1_LEN_TAG + CNP_IPV6_HEADER_TEMPLATE_2_LEN

//17 Bytes
struct roce_options_t {
    //Option - OCS
    OCS_kind:  8;
    OCS_value: 8;
    //Option - TimeStamp
    TS_kind:   8;
    TS_len:    8; //should be set to 10
    TS_value:  32;
    TS_echo:   32;
    //Option - MSS
    MSS_kind:  8;
    MSS_len:   8; //should be set to 4
    MSS_value: 16;
    //Option - EOL
    EOL_kind:  8;
};

// phv 
struct req_tx_phv_t {
    // dma commands
    /* flit 11 */
    dma_cmd16 : 128;
    dma_cmd17 : 128;
    dma_cmd18 : 128;
    dma_cmd19 : 128;


    /* flit 10 */
    dma_cmd12 : 128;
    dma_cmd13 : 128;
    dma_cmd14 : 128;
    dma_cmd15 : 128;

    /* flit 9 */
    dma_cmd8 : 128;
    dma_cmd9 : 128;
    dma_cmd10 : 128;
    dma_cmd11 : 128;

    /* flit 8 */
    dma_cmd4 : 128;
    dma_cmd5 : 128;
    dma_cmd6 : 128;
    dma_cmd7 : 128;
 
    /* flit 7 */
    union {
        struct rrqwqe_t rrqwqe;
        struct {
            inline_data: 256;          // dma_cmd0 // dma_cmd1
            dma_cmd2 : 128;
            dma_cmd3 : 128;
        };
    };

    /* flit 6 */
    union {
        struct {
            struct rdma_cnp_tos_t cnp_tos;                 // 1B
            rrq_p_index: 8;                                //  1B
        };
        struct rdma_cnp_ipv6_header_template cnp_ipv6_hdr;       // 2B
    };
    struct p4plus_to_p4_header_t p4plus_to_p4;      // 20B
    lkey_state: 4;
    lkey_type: 4;
    sqcb0_state_flags: 8;

    union {                                         // 28B
        struct rdma_reth_t reth;                        // 16B
        struct {                                        // 19B
            struct rdma_deth_t deth;                           //  8B
            struct rdma_req_feedback_t rdma_feedback;      // 18B
        };
        struct rdma_cnp_rsvd_t cnp_rsvd;                // 16B
        struct rdma_ieth_t ieth;                        //  4B
        struct rdma_atomiceth_t atomiceth;              // 28B
    };

    struct rdma_bth_t bth;                          // 12B
    // should be from a byte boundary

    /* flit 0-5 */

    // Packet PHV has its own phv_intrinsic space. In order to generate
    // additional RDMA feedback PHV, phv_intrinsic has to be composed. 
    // Since statge2stage space in packet phv is not needed after last
    // stage, this space can be used to compose phv intrinsic for 
    // DMA of RDMA feedback PHV
    union {
        struct {
            struct phv_intr_global_t p4_intr_global;
            struct phv_intr_p4_t p4_intr;
            struct phv_intr_rxdma_t p4_intr_rxdma;
            union {
                struct p4_to_p4plus_roce_header_t p4_to_p4plus;
                struct roce_options_t roce_options;
            };
            zero_pad: 24;
            struct rdma_immeth_t immeth;                    //  4B
            pad5: 40;
            pad4: 512;
            pad3: 512;
            pad2: 512;
            pad1: 512;
            union {
                pad0: 512;
                struct {
                    pad: 256;
                    rrqwqe_base_sges :256;
                };
            };
        };
        // common tx
        struct phv_ common;
    };
};



#if 0
// stage to stage argument structures

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
    struct req_tx_add_hdr_info_t args;
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
    struct req_tx_sqcb0_to_sqcb2_info_t args;
    struct req_tx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct req_tx_credits_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct req_tx_sqcb0_to_credits_info_t args;
    struct req_tx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct req_tx_bktrack_sqcb2_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct req_tx_sqcb0_to_sqcb2_info_t args;
    struct req_tx_to_stage_t to_stage;
    struct phv_global_common_t global;
};
struct req_tx_bktrack_sqpt_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct req_tx_sq_bktrack_info_t args;
    struct req_tx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct req_tx_bktrack_sqwqe_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct req_tx_sq_bktrack_info_t args;
    struct req_tx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct req_tx_bktrack_sqsge_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct req_tx_sq_bktrack_info_t args;
    struct req_tx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct req_tx_bktrack_sqcb2_write_back_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct req_tx_bktrack_sqcb2_write_back_info_t args;
    struct req_tx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

#endif

struct req_tx_ah_size_t {
    size: 8;
    pad: 504;
};

struct req_tx_hdr_template_t {
    data: 512;
};

#endif //__REQ_TX_H
