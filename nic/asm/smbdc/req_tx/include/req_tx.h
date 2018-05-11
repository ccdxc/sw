#ifndef __REQ_TX_H
#define __REQ_TX_H
#include "capri.h"
#include "types.h"
#include "sqcb.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "common_phv.h"

#define REQ_TX_DMA_CMD_PYLD_BASE 4
#define REQ_TX_DMA_CMD_START_FLIT_ID 7
#define REQ_TX_DMA_CMD_START_FLIT_CMD_ID 2
/*
 * DMA command usage plan
 * There are total of 8 DMA commands allowed, 12 to 19 including
 *Incase of req_tx
 * smbdc_send - 12, 13, 14 for rdma_wqe, 
 *              15, 16, 17 for rdma_wqe_ctx, 
 *              18 for rdma doorbell, 
 *              19 for smbdc_context
 * smbdc_mr   - 12, 13, 14, 15 for rdma_wqe
 *              16, 17 - unused
 *              18 for rdma doorbell
 *              19 for smbdc context
 * cq_proxy   - 12-17 unused, 18 for CQ, 19 for RDMA_CQ
 */
#define REQ_TX_DMA_CMD_RDMA_REQ_BASE 12
#define REQ_TX_DMA_CMD_ID_RDMA_DOORBELL 18
#define REQ_TX_DMA_CMD_ID_WQE_CONTEXT 19 
#define REQ_TX_DMA_CMD_ID_CQ 18
#define REQ_TX_DMA_CMD_ID_RDMA_CQ 19 

#define TOTAL_DMA_CMD_BITS 16 * 16 * 8 // (cmds * dma_cmd_size * bits_per_byte) 

//copy from rdma/common/include/types.h

#define RDMA_SQ_RING_ID             0
#define OP_TYPE_SEND                0
#define OP_TYPE_FRMR                12

struct sqwqe_base_t {
    wrid               : 64;
    op_type            : 4;
    complete_notify    : 1;
    fence              : 1;
    solicited_event    : 1;
    inline_data_vld    : 1;
    num_sges           : 8;
    color              : 1;
    rsvd2              : 15;
};

#define FRMR_TYPE_REGULAR 0

// FRMR
struct sqwqe_frmr_t {
    mr_id              : 24;
    type               :  3;
    dma_src_address    : 64;
    num_pages          : 32;
    pad                : 37;
};

// RC send
struct sqwqe_send_t {
    imm_data           : 32;
    inv_key            : 32;
    rsvd1              : 32;
    length             : 32;
    rsvd2              : 32;
};

struct sqsge_t {
    va                 : 64;
    lkey               : 32;
    len                : 32;
};

struct sqwqe_t {
    struct sqwqe_base_t base;
    union {
        //struct sqwqe_atomic_t atomic;
        struct {
            union {
                struct sqwqe_send_t send;
                //struct sqwqe_ud_send_t ud_send;
                //struct sqwqe_write_t write;
                //struct sqwqe_read_t read;
                struct sqwqe_frmr_t frmr;
            };
            //union {
            //    pad : 256;
            //    inline_data: 256;
            //};
            struct sqsge_t sge0;
            struct sqsge_t sge1;
        };
    };
};

//rdma cqwqe

#define     CQ_STATUS_SUCCESS               0
#define     CQ_STATUS_LOCAL_LEN_ERR         1
#define     CQ_STATUS_LOCAL_QP_OPER_ERR     2
#define     CQ_STATUS_LOCAL_PROT_ERR        3
#define     CQ_STATUS_WQE_FLUSHED_ERR       4
#define     CQ_STATUS_MEM_MGMT_OPER_ERR     5
#define     CQ_STATUS_BAD_RESP_ERR          6
#define     CQ_STATUS_LOCAL_ACC_ERR         7
#define     CQ_STATUS_REMOTE_INV_REQ_ERR    8
#define     CQ_STATUS_REMOTE_ACC_ERR        9
#define     CQ_STATUS_REMOTE_OPER_ERR       10
#define     CQ_STATUS_RETRY_EXCEEDED        11
#define     CQ_STATUS_RNR_RETRY_EXCEEDED    12
#define     CQ_STATUS_XRC_VIO_ERR           13

struct cqwqe_t {
    union {
        wrid: 64;
        msn: 32;
    } id;
    op_type: 8;
    status: 8;
    rsvd2: 8;
    qp: 24;
    src_qp: 24;
    smac: 48;
    rkey_inv_vld: 1;
    imm_data_vld: 1;
    color: 1;
    ipv4: 1;
    rsvd1: 4;
    imm_data: 32;
    r_key: 32;
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
    union {
        struct sqwqe_t rdma_wqe0;
        struct {
            dma_cmd8 : 128;
            dma_cmd9 : 128;
            dma_cmd10 : 128;
            dma_cmd11 : 128;
        };
    };

    /* flit 8 */
    union {
        struct sqwqe_t rdma_wqe1;
        struct {
            dma_cmd4 : 128;
            dma_cmd5 : 128;
            dma_cmd6 : 128;
            dma_cmd7 : 128;
        };
    };
 
    /* flit 7 */
    union {
        struct sqwqe_t rdma_wqe2;
        struct {
            dma_cmd0 : 128;
            dma_cmd1 : 128;
            dma_cmd2 : 128;
            dma_cmd3 : 128;
        };
    };

    /* flit 6 */
    struct sqwqe_t rdma_wqe3;

    /* flit 0-5 */
    union {
        struct {
            union {
                pad5: 512;

                // No plan to use t3_s2s in smbdc req_tx.
                // Hence, overloading with other stuff 
                struct rdma_sq_wqe_context_t rdma_wqe_ctx2;
                struct rdma_sq_wqe_context_t rdma_wqe_ctx3;
            };
            union {
                pad4: 512;

                // No plan to use t1_s2s and t2_s2s in smbdc req_tx.
                // Hence, overloading with other stuff 
                struct rdma_sq_wqe_context_t rdma_wqe_ctx0;
                struct rdma_sq_wqe_context_t rdma_wqe_ctx1;
            };
            pad3: 512;
            union {
                pad2: 512;
                struct {
                    pad2_1: 384;
                    // No plan to use to_stage 7 in smbdc req_tx_sq path.
                    // Hence, overloading with other stuff 
                    unused: 64;
                    db_data: 64;
                };
            };
            union {
                pad1: 512;

                // No plan to use to_stage 0, 1 in smbdc req_tx_sq path.
                // to_stage 2, 3 is assumed to be used and unions with pad1_1 below
                // Hence, overloading with other stuff 
                struct {
                    union {
                        struct smbdc_cqe_t smbdc_cqe; //256 bits
                        struct sq_wqe_context_t smbdc_wqe_context; //256 bits
                    };
                    pad1_1: 256;
                };
            };
            pad0: 512;
        };
        // common tx
        struct phv_ common;
    };
};

#endif //__REQ_TX_H
