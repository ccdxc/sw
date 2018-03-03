// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/base.h"
#include "nic/gen/gft/include/p4pd.h"
#include "nic/hal/pd/gft/gft_state.hpp"
#include "nic/hal/pd/hal_pd.hpp"
#include "nic/hal/pd/gft/efe_pd.hpp"
#include "sdk/tcam.hpp"

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
    uint32_t                                idx = 0;

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
    uint32_t                                idx = 0;

    RX_XPOSITION_PGM_WITHID_INIT(0);
    RX_XPOSITION_PGM_WITHID_INIT(1);
    RX_XPOSITION_PGM_WITHID_INIT(2);
    RX_XPOSITION_PGM_WITHID_INIT(3);

end:
    return HAL_RET_OK;
}

hal_ret_t
p4pd_table_defaults_init (p4pd_def_cfg_t *p4pd_def_cfg)
{
    // hack for initial set of DOLs for Uplink -> Uplink
    HAL_ASSERT(p4pd_rx_vport_init() == HAL_RET_OK);

    // Reserve 0th entry
    HAL_ASSERT(p4pd_tx_transp_init() == HAL_RET_OK);

    return HAL_RET_OK;
}

}    // namespace pd
}    // namespace hal

