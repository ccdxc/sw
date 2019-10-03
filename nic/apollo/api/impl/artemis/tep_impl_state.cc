//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains Tunnel EndPoint (TEP) datapath database handling
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/table.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/api/impl/artemis/tep_impl_state.hpp"
#include "gen/p4gen/p4plus_txdma/include/p4plus_txdma_p4pd.h"
#include "gen/p4gen/artemis/include/p4pd.h"

using sdk::table::sdk_table_factory_params_t;

namespace api {
namespace impl {

/// \defgroup PDS_TEP_IMPL_STATE - TEP database functionality
/// \ingroup PDS_TEP
/// \@{

tep_impl_state::tep_impl_state(pds_state *state) {
    p4pd_table_properties_t tinfo;
    sdk_table_factory_params_t table_params;

    // instantiate P4 tables for bookkeeping
    p4pd_global_table_properties_get(P4_P4PLUS_TXDMA_TBL_ID_REMOTE_46_MAPPING,
                                     &tinfo);
    // allocate indexer for remote 4to6 mapping table hw id
    remote_46_tep_idxr_ = indexer::factory(tinfo.tabledepth);
    SDK_ASSERT(remote_46_tep_idxr_ != NULL);

    // TEP1_RX tcam table
    memset(&table_params, 0, sizeof(table_params));
    table_params.table_id = P4TBL_ID_TEP1_RX;
    table_params.entry_trace_en = true;
    tep1_rx_tbl_ = sltcam::factory(&table_params);
    SDK_ASSERT(tep1_rx_tbl_ != NULL);

    // TEP2_RX tcam table
    memset(&table_params, 0, sizeof(table_params));
    table_params.table_id = P4TBL_ID_TEP2_RX;
    table_params.entry_trace_en = true;
    tep2_rx_tbl_ = sltcam::factory(&table_params);
    SDK_ASSERT(tep2_rx_tbl_ != NULL);
}

tep_impl_state::~tep_impl_state() {
    indexer::destroy(remote_46_tep_idxr_);
    sltcam::destroy(tep1_rx_tbl_);
    sltcam::destroy(tep2_rx_tbl_);
}

sdk_ret_t
tep_impl_state::table_transaction_begin(void) {
    tep1_rx_tbl_->txn_start();
    tep2_rx_tbl_->txn_start();
    return SDK_RET_OK;
}

sdk_ret_t
tep_impl_state::table_transaction_end(void) {
    tep1_rx_tbl_->txn_end();
    tep2_rx_tbl_->txn_end();
    return SDK_RET_OK;
}

/// \@}

}    // namespace impl
}    // namespace api
