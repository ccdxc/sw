#ifndef __AQ_TX_H
#define __AQ_TX_H
#include "capri.h"
#include "types.h"
#include "sqcb.h"
#include "cqcb.h"
#include "rqcb.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "common_phv.h"


#define AQ_TX_DMA_CMD_PHV_INTRINSIC 2

#define AQ_TX_MAX_DMA_CMDS        11

#define AQ_TX_DMA_CMD_START_FLIT_ID 9
#define AQ_TX_DMA_CMD_START_FLIT_CMD_ID 1
#define TOTAL_DMA_CMD_BITS 16 * 16 * 8 // (cmds * dma_cmd_size * bits_per_byte) 

#define AQ_TX_DMA_CMD_RDMA_FEEDBACK (AQ_TX_MAX_DMA_CMDS -1)  // This should be at the end
#define AQ_TX_DMA_CMD_RDMA_BUSY     (AQ_TX_MAX_DMA_CMDS -2)  // This should be at the end

/* DMA Cmds for Create CQ */
#define AQ_TX_DMA_CMD_CREATE_CQCB_PT_DST  (AQ_TX_MAX_DMA_CMDS - 3)
#define AQ_TX_DMA_CMD_CREATE_CQCB_PT_SRC  (AQ_TX_MAX_DMA_CMDS - 4 )
#define AQ_TX_DMA_CMD_CREATE_CQ_PT_DST  (AQ_TX_MAX_DMA_CMDS - 5)
#define AQ_TX_DMA_CMD_CREATE_CQ_PT_SRC  (AQ_TX_MAX_DMA_CMDS - 6)
#define AQ_TX_DMA_CMD_CREATE_CQ_CB  (AQ_TX_MAX_DMA_CMDS - 7)

/* DMA Cmds for Create QP */
#define AQ_TX_DMA_CMD_CREATE_QP_CB    (AQ_TX_MAX_DMA_CMDS -3)
#define AQ_TX_DMA_CMD_CREATE_QP_SQPT_DST  (AQ_TX_MAX_DMA_CMDS -4)
#define AQ_TX_DMA_CMD_CREATE_QP_SQPT_SRC  (AQ_TX_MAX_DMA_CMDS -5)

/* DMA Cmds for Modify QP */
// Modify QP to ERR
#define AQ_TX_DMA_CMD_REQ_ERR_FEEDBACK  (AQ_TX_MAX_DMA_CMDS - 3)
#define AQ_TX_DMA_CMD_RESP_ERR_FEEDBACK (AQ_TX_MAX_DMA_CMDS - 4)

// Modify QP Set Header Template
#define AQ_TX_DMA_CMD_MOD_QP_AH_DST    (AQ_TX_MAX_DMA_CMDS - 3)
#define AQ_TX_DMA_CMD_MOD_QP_AH_SRC    (AQ_TX_MAX_DMA_CMDS - 4)
#define AQ_TX_DMA_CMD_CLEAR_STATS_CB   (AQ_TX_MAX_DMA_CMDS -5)

/* DMA Cmds for Create AH */
#define AQ_TX_DMA_CMD_CREATE_AH_DST    (AQ_TX_MAX_DMA_CMDS - 3)
#define AQ_TX_DMA_CMD_CREATE_AH_SRC    (AQ_TX_MAX_DMA_CMDS - 4)
#define AQ_TX_DMA_CMD_CREATE_AH_SIZE   (AQ_TX_MAX_DMA_CMDS - 5)

/* DMA Cmds for AQ Stats Dump */
#define AQ_TX_DMA_CMD_STATS_DUMP_4 (AQ_TX_MAX_DMA_CMDS - 3)
#define AQ_TX_DMA_CMD_STATS_DUMP_3 (AQ_TX_MAX_DMA_CMDS - 4)
#define AQ_TX_DMA_CMD_STATS_DUMP_2 (AQ_TX_MAX_DMA_CMDS - 5)
#define AQ_TX_DMA_CMD_STATS_DUMP_1 (AQ_TX_MAX_DMA_CMDS - 6)

/* 
 * DMA Cmds for Reg MR 
 * 
 * For now we transfer one DMA worth of data: (2047 * 8)
 *
 * TODO: To be implemented later to further optimize
 * With 3 DMA cmds for PT table transfer, we can transfer 6K entries 
 * at a time. That is equal to an MR of size 24MB in one pass. If MR size 
 * is greater then it needs multiple passes.
 */
#define DMA_DATA_SIZE (2047 * 8)
#define DMA_MAX_MAP_COUNT 2047
    
#define AQ_TX_DMA_CMD_MR_PT_DST3 (AQ_TX_MAX_DMA_CMDS - 3)
#define AQ_TX_DMA_CMD_MR_PT_SRC3 (AQ_TX_MAX_DMA_CMDS - 4)
#define AQ_TX_DMA_CMD_MR_PT_DST2 (AQ_TX_MAX_DMA_CMDS - 5)
#define AQ_TX_DMA_CMD_MR_PT_SRC2 (AQ_TX_MAX_DMA_CMDS - 6)
#define AQ_TX_DMA_CMD_MR_PT_DST1 (AQ_TX_MAX_DMA_CMDS - 7)
#define AQ_TX_DMA_CMD_MR_PT_SRC1 (AQ_TX_MAX_DMA_CMDS - 8)
#define AQ_TX_DMA_CMD_MR_KT_UPDATE (AQ_TX_MAX_DMA_CMDS - 9)

#define AQ_TX_CQCB_ADDR_GET(_r, _cqid, _cqcb_base_addr_hi)  \
    CQCB_ADDR_GET(_r, _cqid, _cqcb_base_addr_hi);

#define RDMA_UPDATE_QP_OPER_SET_STATE 0
#define RDMA_UPDATE_QP_OPER_SET_CUR_STATE 1
#define RDMA_UPDATE_QP_OPER_SET_EN_SQD_ASYNC_NOTIFY 2
#define RDMA_UPDATE_QP_OPER_SET_ACCESS_FLAGS 3
#define RDMA_UPDATE_QP_OPER_SET_PKEY_INDEX 4
#define RDMA_UPDATE_QP_OPER_SET_PORT 5
#define RDMA_UPDATE_QP_OPER_SET_QKEY 6
#define RDMA_UPDATE_QP_OPER_SET_AV 7
#define RDMA_UPDATE_QP_OPER_SET_PATH_MTU 8
#define RDMA_UPDATE_QP_OPER_SET_TIMEOUT 9
#define RDMA_UPDATE_QP_OPER_SET_RETRY_CNT 10
#define RDMA_UPDATE_QP_OPER_SET_RNR_RETRY 11
#define RDMA_UPDATE_QP_OPER_SET_RQ_PSN 12
#define RDMA_UPDATE_QP_OPER_SET_MAX_QP_RD_ATOMIC 13
#define RDMA_UPDATE_QP_OPER_SET_ALT_PATH 14
#define RDMA_UPDATE_QP_OPER_SET_MIN_RNR_TIMER 15
#define RDMA_UPDATE_QP_OPER_SET_SQ_PSN 16
#define RDMA_UPDATE_QP_OPER_SET_MAX_DEST_RD_ATOMIC 17
#define RDMA_UPDATE_QP_OPER_SET_PATH_MIG_STATE 18
#define RDMA_UPDATE_QP_OPER_SET_CAP 19
#define RDMA_UPDATE_QP_OPER_SET_DEST_QPN 20
#define RDMA_UPDATE_QP_OPER_SET_RESERVED1 21
#define RDMA_UPDATE_QP_OPER_SET_RESERVED2 22
#define RDMA_UPDATE_QP_OPER_SET_RESERVED3 23
#define RDMA_UPDATE_QP_OPER_SET_RESERVED4 24
#define RDMA_UPDATE_QP_OPER_SET_RATE_LIMIT 25
 
struct aq_tx_dma_cmds_flit_t {
    dma_cmd0 : 128;
    dma_cmd1 : 128;
    dma_cmd2 : 128;
    dma_cmd3 : 128;
};

struct err_feedback_t {
    struct phv_intr_global_t p4_intr_global;
    struct phv_intr_p4_t p4_intr;
    struct phv_intr_rxdma_t p4_intr_rxdma;
    struct p4_to_p4plus_roce_header_t p4_to_p4plus;
    struct rdma_feedback_t rdma_feedback;
    pad     : 8;
};

// phv 
struct aq_tx_phv_t {
    // dma commands
    /* flit 11 */
    struct aq_tx_dma_cmds_flit_t flit_11;
    
        /* flit 10 */
    struct aq_tx_dma_cmds_flit_t flit_10;
    
        /* flit 9 */
    union {
        struct aq_tx_dma_cmds_flit_t flit_9;
        struct {
            proxy_cindex: 16;
            map_count_completed: 32;
            first_pass: 8;
            busy: 8;
            pad: 64;
            dma_cmds_1_2_3: 384;
        };
    };
    
        /* flit 8 */
    union {
        struct aq_tx_dma_cmds_flit_t flit_8;
        struct sqcb2_t sqcb2;
        struct err_feedback_t req_feedback;
    };
     
        /* flit 7 */
    union {
        struct aq_tx_dma_cmds_flit_t flit_7;
        struct sqcb1_t sqcb1;
        struct sqcb4_t sqcb4;
        struct err_feedback_t resp_feedback;
    };
        
        /* flit 6 */
    union {
        struct aq_tx_dma_cmds_flit_t flit_6;
        struct key_entry_t key;
        struct sqcb0_t sqcb0;
        struct cqcb_t  cqcb;
        struct sqcb3_t sqcb3;
        ah_size :8;
    };

    /* flit 0-5 */
    union {
        struct {
            struct phv_intr_p4_t p4_intr;            //  5 Bytes
            struct phv_intr_rxdma_t p4_intr_rxdma;   // 10 Bytes
            struct aq_p4_to_p4plus_roce_header_t p4_to_p4plus; // 20 Bytes
            struct rdma_aq_feedback_t rdma_feedback;    // 29 Bytes
            pad4: 512;
            pad3: 512;
            pad2: 512;
            pad1: 512;
            pad0: 512;
        };
        // common tx
        struct phv_ common;
    };
};
#endif //__AQ_TX_H
