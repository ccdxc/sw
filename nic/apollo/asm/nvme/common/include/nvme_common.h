#ifndef __NVME_COMMON_H
#define __NVME_COMMON_H

#include "capri.h"

#define LOG_NUM_PRP_BYTES       3   //2^3 = 8
#define LOG_NUM_PAGE_PTR_BYTES  3   //2^3 = 8

//ring pindex/cindex
#define SQ_P_INDEX              d.{pi_0}.hx
#define SQ_C_INDEX              d.{ci_0}.hx
#define CMDID_RING_PROXY_CI     d.{cmdid_ring_proxy_ci}.hx
#define CMDID_RING_PROXY_CI_LE  d.{cmdid_ring_proxy_ci}
#define CMDID_RING_PI           d.{cmdid_ring_pi}.hx
#define CMDID_RING_CI           d.{cmdid_ring_ci}.hx
#define CMDID                   d.{cmdid}.hx
#define XTS_Q_PI                d.{xts_q_pi}.hx
#define SESSPREXTSTX_P_INDEX    d.{pi_0}.hx
#define SESSPREXTSTX_C_INDEX    d.{ci_0}.hx
#define SESSPOSTXTSTX_P_INDEX   d.{pi_1}.hx
#define SESSPOSTXTSTX_C_INDEX   d.{ci_1}.hx
#define NMDPR_IDX               d.{idx}.wx

//cb, ring entry sizes
#define LOG_SQE_SIZE            6   //2^6 = 64
#define LOG_CQE_SIZE            4   //2^4 = 16
#define LOG_NSCB_SIZE           6   //2^6 = 64
#define LOG_TXSESSPRODCB_SIZE   6   //2^6 = 64

#define LOG_CMDID_SIZE              1   //2^1 = 2
#define LOG_CMDID_RING_ENTRY_SIZE   LOG_CMDID_SIZE
#define LOG_CMD_CTXT_SIZE           11  //2^11=2K

#define LOG_SESSXTS_Q_ENTRY_SIZE    LOG_CMDID_SIZE
#define LOG_XTS_DESC_SIZE       7   //2^7 = 128
#define LOG_AOL_SIZE            6   //2^6 = 64
#define AOL_SIZE                (1 << (LOG_AOL_SIZE))
#define LOG_AOL_PAIR_SIZE       7   //2^7 = 128
#define LOG_IV_SIZE             4   //2^4 = 16

#define LOG_NMDPR_RING_SIZE     (CAPRI_TNMDPR_BIG_RING_SHIFT)
#define NMDPR_RING_SIZE         (1 << LOG_NMDPR_RING_SIZE)
#define NMDPR_RING_SIZE_MASK    (NMDPR_RING_SIZE-1)
#define LOG_NMDPR_RING_ENTRY_SIZE   3
#define NMDPR_RING_ENTRY_SIZE   (1 << LOG_NMDPR_RING_ENTRY_SIZE)

//rings, their ids and priorities
//sq
#define MAX_REQ_TX_RINGS        1

#define SQ_PRI                  0

#define SQ_RING_ID              0

//sessxtstx
#define MAX_SESSXTS_TX_RINGS    2

#define SESSPREXTS_TX_PRI       1
#define SESSPOSTXTS_TX_PRI      0

#define SESSPREXTS_TX_RING_ID   0
#define SESSPOSTXTS_TX_RING_ID  1

//sessdgsttx
#define MAX_SESSDGST_TX_RINGS   2

#define SESSPREDGST_TX_PRI      1
#define SESSPOSTDGST_TX_PRI     0

#define SESSPREDGST_TX_RING_ID   0
#define SESSPOSTDGST_TX_RING_ID  1

//globals
#define K_GLOBAL_LIF            k.phv_global_common_lif
#define K_GLOBAL_QTYPE          k.phv_global_common_qtype
#define K_GLOBAL_QID            k.phv_global_common_qid
#define K_GLOBAL_CB_ADDR        k.phv_global_cb_addr
#define K_GLOBAL_LIF_QID        k.{phv_global_common_lif...phv_global_common_qid}

//qtypes
#define NVME_QTYPE_SQ           0
#define NVME_QTYPE_CQ           1
#define NVME_QTYPE_ARMQ         2
#define NVME_QTYPE_SESSXTSTX    3
#define NVME_QTYPE_SESSDGSTTX   4
#define NVME_QTYPE_SESSXTSRX    5
#define NVME_QTYPE_SESSDGSTRX   6

//dma cmd ptrs
#define NVME_REQ_TX_DMA_CMD_PTR (PHV_FIELD_START_OFFSET(cmd_ctxt_dma_dma_cmd_type)/16)
#define NVME_SESSPREXTSTX_DMA_CMD_PTR (PHV_FIELD_START_OFFSET(pkt_desc_dma_dma_cmd_type)/16)

//command context offsets
#define NVME_CMD_CTXT_HDR_SIZE          64
#define NVME_CMD_CTXT_PRP_LIST_OFFSET   NVME_CMD_CTXT_HDR_SIZE
#define NVME_CMD_CTXT_MAX_PRPS          40  // ideally we need 33 prps, but aligning
                                            // to next cache line boundary
#define NVME_CMD_CTXT_PRP_LIST_SIZE     (NVME_CMD_CTXT_MAX_PRPS << LOG_NUM_PRP_BYTES)
#define NVME_CMD_CTXT_PAGE_LIST_OFFSET  (NVME_CMD_CTXT_HDR_SIZE + NVME_CMD_CTXT_PRP_LIST_SIZE)

#define NVME_CMD_CTXT_MAX_PAGE_PTRS     16
#define NVME_CMD_CTXT_PAGE_LIST_SIZE    (NVME_CMD_CTXT_MAX_PAGE_PTRS << LOG_NUM_PAGE_PTR_BYTES)

//XTS related defines
#define HW_XTS_TX_DB_ADDR        CAPRI_BARCO_MD_HENS_REG_XTS0_PRODUCER_IDX
#define HW_XTS_RX_DB_ADDR        CAPRI_BARCO_MD_HENS_REG_XTS1_PRODUCER_IDX

#define HW_CMD_XTS_ENCRYPT      0x40000000
#define HW_CMD_XTS_DECRYPT      0x40100000

//Semaphores
#define NVME_TX_NMDPR_ALLOC_IDX CAPRI_SEM_TNMDPR_BIG_ALLOC_INF_ADDR
#define NVME_TX_NMDPR_FREE_IDX  CAPRI_SEM_TNMDPR_BIG_FREE_INC_ADDR

#define NVME_RX_NMDPR_ALLOC_IDX CAPRI_SEM_RNMDPR_BIG_ALLOC_INF_ADDR
#define NVME_RX_NMDPR_FREE_IDX  CAPRI_SEM_RNMDPR_BIG_FREE_INC_ADDR

#define NVME_LOG_MAX_DATA_SIZE_IN_PAGE  13  // 2^13 = 8K
#define NVME_MAX_DATA_SIZE_IN_PAGE      (1 << NVME_LOG_MAX_DATA_SIZE_IN_PAGE)
#define NVME_MAX_DATA_SIZE_IN_PAGE_MASK (NVME_MAX_DATA_SIZE_IN_PAGE - 1)

#define NVME_PAGE_DATA_LEN_SHIFT    48

//NVME-over-TCP related defines

#define PKT_DESC_SCRATCH_OVERHEAD           64
#define PKT_DESC_AOL_OVERHEAD               64
#define PKT_DESC_OVERHEAD                   (PKT_DESC_SCRATCH_OVERHEAD + PKT_DESC_AOL_OVERHEAD) //  (scratch + AOL)
#define NVME_O_TCP_CH_SIZE                  8
#define NVME_O_TCP_PSH_CMD_CAPSULE_SIZE     64
#define NVME_O_TCP_HDGST_SIZE               4
#define NVME_O_TCP_CMD_CAPSULE_HEADER_SIZE  \
    (NVME_O_TCP_CH_SIZE + NVME_O_TCP_PSH_CMD_CAPSULE_SIZE + NVME_O_TCP_HDGST_SIZE)
#define NVME_O_TCP_DDGST_SIZE               4


#endif //__NVME_COMMON_H
