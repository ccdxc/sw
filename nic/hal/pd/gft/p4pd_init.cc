// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/base.h"
#include "nic/gen/gft/include/p4pd.h"
#include "nic/hal/pd/gft/gft_state.hpp"
#include "nic/hal/pd/hal_pd.hpp"
#include "nic/hal/pd/gft/efe_pd.hpp"
#include "nic/hal/pd/gft/p4pd_defaults.hpp"
#include "sdk/tcam.hpp"
#include "nic/p4/rdma/include/rdma_defines.h"
#include "nic/p4/include/common_defines.h"
#include "nic/p4/gft/include/defines.h"

using sdk::table::tcam;

namespace hal {
namespace pd {

static hal_ret_t
p4pd_rx_vport_init (void)
{
    hal_ret_t               ret;
    sdk_ret_t               sdk_ret;
    uint32_t                idx;
    rx_vport_swkey_t        key;
    rx_vport_swkey_mask_t   mask;
    rx_vport_actiondata     data;
    tcam                    *rx_vport;

    rx_vport = g_hal_state_pd->tcam_table(P4TBL_ID_RX_VPORT);
    HAL_ASSERT(rx_vport!= NULL);

    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    data.rx_vport_action_u.rx_vport_rx_vport.tm_oport = TM_PORT_DMA;

    sdk_ret = rx_vport->insert(&key, &mask, &data, &idx, false);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("rx_vport tcam write failure, "
                      "idx : {}, err : {}", idx, ret);
        return ret;
    }
    HAL_TRACE_DEBUG("rx_vport tcam write, "
                    "idx : {}, ret: {}", idx, ret);

    return ret;
}

#define TX_XPOSITION_PGM_WITHID_INIT(TBL_ID)                                \
    tx_xpos_tbl = g_hal_state_pd->                                          \
                  dm_table(P4TBL_ID_TX_HDR_TRANSPOSITIONS ## TBL_ID);       \
    sdk_ret = tx_xpos_tbl->insert_withid(&xpo ## TBL_ID ## _data,           \
                                         idx);                              \
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);                                  \
    if (ret != HAL_RET_OK) {                                                \
         HAL_TRACE_ERR(" Failed to insert into tx hdr xpos" #TBL_ID " table"\
                       "err : {}", ret);                                    \
        goto end;                                                           \
    }                                                                       \
    HAL_TRACE_DEBUG("Programmed tx_hdr_xpos" #TBL_ID " at {}",              \
                    idx);                                                   \

//-----------------------------------------------------------------------------
// TX Transpositions 
//  0: nop entry
//-----------------------------------------------------------------------------
static hal_ret_t
p4pd_tx_transp_init (void)
{
    hal_ret_t                               ret;
    sdk_ret_t                               sdk_ret;
    tx_hdr_transpositions0_actiondata       xpo0_data = { 0 }; 
    tx_hdr_transpositions1_actiondata       xpo1_data = { 0 }; 
    tx_hdr_transpositions2_actiondata       xpo2_data = { 0 }; 
    tx_hdr_transpositions3_actiondata       xpo3_data = { 0 }; 
    directmap                               *tx_xpos_tbl = NULL;
    uint32_t                                idx = TX_TRANSPOSITION_NOP_ENTRY;

    TX_XPOSITION_PGM_WITHID_INIT(0);
    TX_XPOSITION_PGM_WITHID_INIT(1);
    TX_XPOSITION_PGM_WITHID_INIT(2);
    TX_XPOSITION_PGM_WITHID_INIT(3);

end:
    return HAL_RET_OK;
}

#define RX_XPOSITION_PGM_WITHID_INIT(TBL_ID)                                \
    rx_xpos_tbl = g_hal_state_pd->                                          \
                  dm_table(P4TBL_ID_RX_HDR_TRANSPOSITIONS ## TBL_ID);       \
    sdk_ret = rx_xpos_tbl->insert_withid(&xpo ## TBL_ID ## _data,           \
                                         idx);                              \
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);                                  \
    if (ret != HAL_RET_OK) {                                                \
         HAL_TRACE_ERR(" Failed to insert into rx hdr xpos" #TBL_ID " table"\
                       "err : {}", ret);                                    \
        goto end;                                                           \
    }                                                                       \
    HAL_TRACE_DEBUG("Programmed rx_hdr_xpos" #TBL_ID " at {}",              \
                    idx);                                                   \
//-----------------------------------------------------------------------------
// RX Transpositions 
//  0: nop entry
//-----------------------------------------------------------------------------
static hal_ret_t
p4pd_rx_transp_init (void)
{
    hal_ret_t                               ret;
    sdk_ret_t                               sdk_ret;
    rx_hdr_transpositions0_actiondata       xpo0_data = { 0 }; 
    rx_hdr_transpositions1_actiondata       xpo1_data = { 0 }; 
    rx_hdr_transpositions2_actiondata       xpo2_data = { 0 }; 
    rx_hdr_transpositions3_actiondata       xpo3_data = { 0 }; 
    directmap                               *rx_xpos_tbl = NULL;
    uint32_t                                idx = RX_TRANSPOSITION_NOP_ENTRY;

    RX_XPOSITION_PGM_WITHID_INIT(0);
    RX_XPOSITION_PGM_WITHID_INIT(1);
    RX_XPOSITION_PGM_WITHID_INIT(2);
    RX_XPOSITION_PGM_WITHID_INIT(3);

end:
    return HAL_RET_OK;
}

#define DECODE_ROCE_OPCODE_TABLE_SIZE       256
#define PARSED_HDRS_LEN_BTH                 (sizeof(rdma_bth_t) + 12)
#define PARSED_HDRS_LEN_BTH_DETH            (sizeof(rdma_bth_t) + sizeof(rdma_deth_t) + 12)
#define PARSED_HDRS_LEN_BTH_DETH_IMMDT      (sizeof(rdma_bth_t) + sizeof(rdma_deth_t) + sizeof(rdma_immeth_t) + 12)

typedef struct roce_opcode_info_t {
    uint32_t valid:1;
    uint32_t roce_hdr_length: 8; //in bytes
    uint32_t parsed_hdrs_len:8;
    uint32_t type: 4; //LIF sub-type
    uint32_t raw_flags:16;
} roce_opcode_info_t;

roce_opcode_info_t opc_to_info[DECODE_ROCE_OPCODE_TABLE_SIZE] = {
    //Reliable-Connect opcodes
    {1, sizeof(rdma_bth_t), PARSED_HDRS_LEN_BTH, Q_TYPE_RDMA_RQ,
     (RESP_RX_FLAG_FIRST | RESP_RX_FLAG_SEND)}, //0 - send-first
    {1, sizeof(rdma_bth_t), PARSED_HDRS_LEN_BTH, Q_TYPE_RDMA_RQ,
     (RESP_RX_FLAG_MIDDLE | RESP_RX_FLAG_SEND)}, //1 - send-middle
    {1, sizeof(rdma_bth_t), PARSED_HDRS_LEN_BTH, Q_TYPE_RDMA_RQ,
     (RESP_RX_FLAG_LAST | RESP_RX_FLAG_SEND | RESP_RX_FLAG_COMPLETION)}, //2 - send-last
    {1, sizeof(rdma_bth_t)+sizeof(rdma_immeth_t), PARSED_HDRS_LEN_BTH, Q_TYPE_RDMA_RQ,
     (RESP_RX_FLAG_LAST | RESP_RX_FLAG_SEND | RESP_RX_FLAG_IMMDT | RESP_RX_FLAG_COMPLETION)}, //3 - send-last-with-immediate
    {1, sizeof(rdma_bth_t), PARSED_HDRS_LEN_BTH, Q_TYPE_RDMA_RQ,
     (RESP_RX_FLAG_ONLY | RESP_RX_FLAG_SEND | RESP_RX_FLAG_COMPLETION)}, //4 - send-only
    {1, sizeof(rdma_bth_t)+sizeof(rdma_immeth_t), PARSED_HDRS_LEN_BTH, Q_TYPE_RDMA_RQ,
     (RESP_RX_FLAG_ONLY | RESP_RX_FLAG_SEND | RESP_RX_FLAG_IMMDT | RESP_RX_FLAG_COMPLETION)}, //5 - send-only-with-immediate
    {1, sizeof(rdma_bth_t)+sizeof(rdma_reth_t), PARSED_HDRS_LEN_BTH, Q_TYPE_RDMA_RQ,
     (RESP_RX_FLAG_FIRST | RESP_RX_FLAG_WRITE)}, //6 - write-first
    {1, sizeof(rdma_bth_t), PARSED_HDRS_LEN_BTH, Q_TYPE_RDMA_RQ,
     (RESP_RX_FLAG_MIDDLE | RESP_RX_FLAG_WRITE)}, //7 - write-middle
    {1, sizeof(rdma_bth_t), PARSED_HDRS_LEN_BTH, Q_TYPE_RDMA_RQ,
     (RESP_RX_FLAG_LAST | RESP_RX_FLAG_WRITE)}, //8 - write-last
    {1, sizeof(rdma_bth_t)+sizeof(rdma_immeth_t), PARSED_HDRS_LEN_BTH, Q_TYPE_RDMA_RQ,
     (RESP_RX_FLAG_LAST | RESP_RX_FLAG_WRITE | RESP_RX_FLAG_IMMDT | RESP_RX_FLAG_COMPLETION)},//9 - write-last-with-immediate
    {1, sizeof(rdma_bth_t)+sizeof(rdma_reth_t), PARSED_HDRS_LEN_BTH, Q_TYPE_RDMA_RQ,
     (RESP_RX_FLAG_ONLY | RESP_RX_FLAG_WRITE)}, //10 - write-only
    {1, sizeof(rdma_bth_t)+sizeof(rdma_reth_t)+sizeof(rdma_immeth_t), PARSED_HDRS_LEN_BTH, Q_TYPE_RDMA_RQ,
     (RESP_RX_FLAG_ONLY | RESP_RX_FLAG_WRITE | RESP_RX_FLAG_IMMDT | RESP_RX_FLAG_COMPLETION)}, //11 - write-only-with-immediate
    {1, sizeof(rdma_bth_t)+sizeof(rdma_reth_t), PARSED_HDRS_LEN_BTH, Q_TYPE_RDMA_RQ,
     (RESP_RX_FLAG_READ_REQ)}, //12 - read-request
    {1, sizeof(rdma_bth_t)+sizeof(rdma_aeth_t), PARSED_HDRS_LEN_BTH, Q_TYPE_RDMA_SQ,
     (REQ_RX_FLAG_FIRST | REQ_RX_FLAG_READ_RESP | REQ_RX_FLAG_AETH | REQ_RX_FLAG_COMPLETION)}, //13 - read-response-first
    {1, sizeof(rdma_bth_t), PARSED_HDRS_LEN_BTH, Q_TYPE_RDMA_SQ,
     (REQ_RX_FLAG_MIDDLE | REQ_RX_FLAG_READ_RESP)}, //14 - read-response-middle
    {1, sizeof(rdma_bth_t)+sizeof(rdma_aeth_t), PARSED_HDRS_LEN_BTH, Q_TYPE_RDMA_SQ,
     (REQ_RX_FLAG_LAST | REQ_RX_FLAG_READ_RESP | REQ_RX_FLAG_AETH | REQ_RX_FLAG_COMPLETION)}, //15 - read-response-last
    {1, sizeof(rdma_bth_t)+sizeof(rdma_aeth_t), PARSED_HDRS_LEN_BTH, Q_TYPE_RDMA_SQ,
     (REQ_RX_FLAG_ONLY | REQ_RX_FLAG_READ_RESP | REQ_RX_FLAG_AETH | REQ_RX_FLAG_COMPLETION)}, //16 - read-response-only
    {1, sizeof(rdma_bth_t)+sizeof(rdma_aeth_t), PARSED_HDRS_LEN_BTH, Q_TYPE_RDMA_SQ,
     (REQ_RX_FLAG_AETH | REQ_RX_FLAG_ACK | REQ_RX_FLAG_COMPLETION)}, //17 - ack
    {1, sizeof(rdma_bth_t)+sizeof(rdma_aeth_t)+sizeof(rdma_atomicaeth_t), PARSED_HDRS_LEN_BTH, Q_TYPE_RDMA_SQ,
     (REQ_RX_FLAG_AETH | REQ_RX_FLAG_ATOMIC_AETH | REQ_RX_FLAG_COMPLETION)}, //18 - atomic-ack
    {1, sizeof(rdma_bth_t)+sizeof(rdma_atomiceth_t), PARSED_HDRS_LEN_BTH, Q_TYPE_RDMA_RQ,
     (RESP_RX_FLAG_ATOMIC_CSWAP)}, //19 - compare-and-swap
    {1, sizeof(rdma_bth_t)+sizeof(rdma_atomiceth_t), PARSED_HDRS_LEN_BTH, Q_TYPE_RDMA_RQ,
     (RESP_RX_FLAG_ATOMIC_FNA)}, //20 - fetch-and-add
    {0, 0, 0, 0}, //21 - Reserved
    {1, sizeof(rdma_bth_t)+sizeof(rdma_ieth_t), PARSED_HDRS_LEN_BTH, Q_TYPE_RDMA_RQ,
     (RESP_RX_FLAG_LAST | RESP_RX_FLAG_SEND | RESP_RX_FLAG_COMPLETION | RESP_RX_FLAG_INV_RKEY)}, //22 - send-last-with-inv-rkey
    {1, sizeof(rdma_bth_t)+sizeof(rdma_ieth_t), PARSED_HDRS_LEN_BTH, Q_TYPE_RDMA_RQ,
     (RESP_RX_FLAG_ONLY | RESP_RX_FLAG_SEND | RESP_RX_FLAG_COMPLETION | RESP_RX_FLAG_INV_RKEY)}, //23 - send-only-with-inv-rkey
};

static hal_ret_t
p4pd_decode_roce_opcode_init (void)
{
    uint32_t                     idx = 0;
    hal_ret_t                    ret;
    sdk_ret_t                    sdk_ret;
    directmap                    *dm;
    rx_roce_actiondata data = { 0 };

    // C++ compiler did not allow sparse initialization. compiler must be old.
    // So lets initialize the for UD entries here.

    opc_to_info[100].valid = 1;
    opc_to_info[100].roce_hdr_length = sizeof(rdma_bth_t)+sizeof(rdma_deth_t);
    opc_to_info[100].type = Q_TYPE_RDMA_RQ;
    opc_to_info[100].raw_flags = (RESP_RX_FLAG_ONLY | RESP_RX_FLAG_SEND |
                                  RESP_RX_FLAG_COMPLETION|RESP_RX_FLAG_UD);
    opc_to_info[100].parsed_hdrs_len = PARSED_HDRS_LEN_BTH_DETH;

    opc_to_info[101].valid = 1;
    opc_to_info[101].roce_hdr_length = sizeof(rdma_bth_t)+sizeof(rdma_deth_t)+sizeof(rdma_immeth_t);
    opc_to_info[101].type = Q_TYPE_RDMA_RQ;
    opc_to_info[101].raw_flags = (RESP_RX_FLAG_ONLY | RESP_RX_FLAG_SEND |
                                  RESP_RX_FLAG_IMMDT | RESP_RX_FLAG_COMPLETION |
                                  RESP_RX_FLAG_UD);
    opc_to_info[101].parsed_hdrs_len = PARSED_HDRS_LEN_BTH_DETH_IMMDT;

    // CNP packet entry.
    opc_to_info[129].valid = 1;
    opc_to_info[129].roce_hdr_length = sizeof(rdma_bth_t)+sizeof(rdma_reth_t);
    opc_to_info[129].type = Q_TYPE_RDMA_RQ;
    opc_to_info[129].roce_hdr_length = sizeof(rdma_bth_t);
    opc_to_info[129].parsed_hdrs_len = PARSED_HDRS_LEN_BTH;

    dm = g_hal_state_pd->dm_table(P4TBL_ID_RX_ROCE);
    HAL_ASSERT(dm != NULL);

    for (idx = 0; idx < DECODE_ROCE_OPCODE_TABLE_SIZE; idx++) {

        if (opc_to_info[idx].valid == 1) {
            // valid entry
            data.actionid = RX_ROCE_RX_ROCE_ID;
            data.rx_roce_action_u.rx_roce_rx_roce.qtype =
                opc_to_info[idx].type;
            data.rx_roce_action_u.rx_roce_rx_roce.len =
                opc_to_info[idx].roce_hdr_length;
            data.rx_roce_action_u.rx_roce_rx_roce.raw_flags = opc_to_info[idx].raw_flags;
            data.rx_roce_action_u.rx_roce_rx_roce.parsed_hdrs_len =
                opc_to_info[idx].parsed_hdrs_len;
        } else {
            //Except for invalid opcode 32(error test case), skip all others for now, to save HAL bootup time
            if (idx != 32) {
                continue;
            }
            data.actionid = RX_ROCE_RX_ROCE_ID;
            data.rx_roce_action_u.rx_roce_rx_roce.qtype = Q_TYPE_RDMA_RQ;
            data.rx_roce_action_u.rx_roce_rx_roce.len = sizeof(rdma_bth_t);
            data.rx_roce_action_u.rx_roce_rx_roce.raw_flags = 0;
            data.rx_roce_action_u.rx_roce_rx_roce.parsed_hdrs_len =
                PARSED_HDRS_LEN_BTH;
        }
        HAL_TRACE_DEBUG("Inserting opcode id {}, type {}, raw_flags {}, len {}",
                        idx, opc_to_info[idx].type, opc_to_info[idx].raw_flags,
                        opc_to_info[idx].roce_hdr_length);
        sdk_ret = dm->insert_withid(&data, idx);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("decode roce opcode table write failure, idx : {}, err : {}",
                          idx, ret);
            return ret;
        }
    }

    return HAL_RET_OK;
}

hal_ret_t
p4pd_table_defaults_init (p4pd_def_cfg_t *p4pd_def_cfg)
{
    // hack for initial set of DOLs for Uplink -> Uplink
    HAL_ASSERT(p4pd_rx_vport_init() == HAL_RET_OK);

    // Reserve 0th entry for tx and rx transposition tables
    HAL_ASSERT(p4pd_tx_transp_init() == HAL_RET_OK);
    HAL_ASSERT(p4pd_rx_transp_init() == HAL_RET_OK);

    HAL_ASSERT(p4pd_decode_roce_opcode_init() == HAL_RET_OK);

    return HAL_RET_OK;
}

}    // namespace pd
}    // namespace hal

