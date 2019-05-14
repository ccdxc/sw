#ifndef __NVME_COMMON_H
#define __NVME_COMMON_H

#include "capri.h"

//ring pindex/cindex
#define SQ_P_INDEX              d.{pi_0}.hx
#define SQ_C_INDEX              d.{ci_0}.hx
#define CMDID_RING_PROXY_CI     d.{cmdid_ring_proxy_ci}.hx
#define CMDID_RING_PROXY_CI_LE  d.{cmdid_ring_proxy_ci}
#define CMDID_RING_PI           d.{cmdid_ring_pi}.hx
#define CMDID_RING_CI           d.{cmdid_ring_ci}.hx
#define CMDID                   d.{cmdid}.hx
#define XTS_Q_PI                d.{xts_q_pi}.hx

//cb, ring entry sizes
#define LOG_SQE_SIZE            6   //2^6 = 64
#define LOG_CQE_SIZE            4   //2^4 = 16
#define LOG_NSCB_SIZE           6   //2^6 = 64
#define LOG_TXSESSPRODCB_SIZE   6   //2^6 = 64

#define LOG_CMDID_RING_ENTRY_SIZE   1   //2^1 = 2

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
#define NVME_REQ_TX_DMA_CMD_PTR (PHV_FIELD_START_OFFSET(wqe_dma_dma_cmd_type)/16)
#endif //__NVME_COMMON_H
