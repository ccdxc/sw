//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This library provides p4 Ethernet table programming API's
///
//----------------------------------------------------------------------------

#ifndef __ETH_P4PD_HPP__
#define __ETH_P4PD_HPP__

#include "include/sdk/base.hpp"

// TODO: what was done here was clearly a hack, please clean up !!!
#if defined(APOLLO) || defined(ARTEMIS) || defined(APULU) || defined(ATHENA)
#include "gen/p4gen/p4plus_rxdma/include/p4plus_rxdma_p4pd.h"
#include "gen/p4gen/p4plus_rxdma/include/p4plus_rxdma_p4pd_table.h"
#include "gen/p4gen/p4plus_txdma/include/p4plus_txdma_p4pd.h"
#include "gen/p4gen/p4plus_txdma/include/p4plus_txdma_p4pd_table.h"
#else
#include "gen/p4gen/common_rxdma_actions/include/common_rxdma_actions_p4pd.h"
#include "gen/p4gen/common_rxdma_actions/include/common_rxdma_actions_p4pd_table.h"
#include "gen/p4gen/common_txdma_actions/include/common_txdma_actions_p4pd.h"
#include "gen/p4gen/common_txdma_actions/include/common_txdma_actions_p4pd_table.h"
#endif

#if defined(APOLLO) || defined(ARTEMIS) || defined(APULU) || defined(ATHENA)
#define P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN            P4_P4PLUS_RXDMA_TBL_ID_INDEX_MIN
#define P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MAX            P4_P4PLUS_RXDMA_TBL_ID_INDEX_MAX
#define P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN            P4_P4PLUS_TXDMA_TBL_ID_INDEX_MIN
#define P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MAX            P4_P4PLUS_TXDMA_TBL_ID_INDEX_MAX
#define P4_COMMON_RXDMA_ACTIONS_TBL_ID_TBLMIN               P4_P4PLUS_RXDMA_TBL_ID_TBLMIN
#define P4_COMMON_RXDMA_ACTIONS_TBL_ID_TBLMAX               P4_P4PLUS_RXDMA_TBL_ID_TBLMAX
#define P4_COMMON_TXDMA_ACTIONS_TBL_ID_TBLMIN               P4_P4PLUS_TXDMA_TBL_ID_TBLMIN
#define P4_COMMON_TXDMA_ACTIONS_TBL_ID_TBLMAX               P4_P4PLUS_TXDMA_TBL_ID_TBLMAX
#define P4_COMMON_RXDMA_ACTIONS_TBL_ID_ETH_RX_RSS_PARAMS    P4_P4PLUS_RXDMA_TBL_ID_ETH_RX_RSS_PARAMS
#endif

// Maximum number of queue per LIF
#define ETH_RSS_MAX_QUEUES                 (128)

// Number of entries in a LIF's indirection table
#define ETH_RSS_LIF_INDIR_TBL_LEN          ETH_RSS_MAX_QUEUES

// Size of each LIF indirection table entry
#define ETH_RSS_LIF_INDIR_TBL_ENTRY_SZ     \
    (sizeof(eth_rx_rss_indir_eth_rx_rss_indir_t))

// Size of a LIF's indirection table
#define ETH_RSS_LIF_INDIR_TBL_SZ           \
    (ETH_RSS_LIF_INDIR_TBL_LEN * ETH_RSS_LIF_INDIR_TBL_ENTRY_SZ)

// Max number of LIFs supported
#define MAX_LIFS                           (2048)

// Size of the entire LIF indirection table
#define ETH_RSS_INDIR_TBL_SZ               (MAX_LIFS * ETH_RSS_LIF_INDIR_TBL_SZ)

/// \brief    Program RSS p4 tables
/// \param[in] hw_lif_id hardware lif id
/// \param[in] rss_type  rss type information
/// \param[in] rss_key   rss key data
/// \return    #SDK_RET_OK on success, failure status code on error
sdk_ret_t p4plus_rxdma_rss_params_update(
    uint32_t hw_lif_id, uint8_t rss_type, uint8_t *rss_key, bool debug);

/// \brief    Get RSS p4 table entries
/// \param[in] hw_lif_id hardware lif id
/// \param[in] data      Rss data
/// \return    #SDK_RET_OK on success, failure status code on error
sdk_ret_t p4plus_rxdma_rss_params_get(
    uint32_t hw_lif_id, eth_rx_rss_params_actiondata_t *data);

/// \brief    Program RSS indirection tables
/// \param[in] hw_lif_id hardware lif id
/// \param[in] index     indirect table index
/// \param[in] enable    enable indirect table index
/// \param[in] qid       RSS qid
/// \return    #SDK_RET_OK on success, failure status code on error
sdk_ret_t p4plus_rxdma_rss_indir_update(
    uint32_t hw_lif_id, uint8_t index, uint8_t enable, uint8_t qid);

/// \brief    create security profile
/// \param[in] hw_lif_id hardware lif id
/// \param[in] index     indirect table index
/// \param[in] data      Rss indirect table data
/// \return    #SDK_RET_OK on success, failure status code on error
sdk_ret_t p4plus_rxdma_rss_indir_get(
    uint32_t hw_lif_id, uint8_t index, eth_rx_rss_indir_actiondata_t *data);

#endif    // __ETH_P4PD_HPP__
