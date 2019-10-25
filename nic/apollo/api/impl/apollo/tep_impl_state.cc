//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains Tunnel EndPoint (TEP) datapath database handling
///
//----------------------------------------------------------------------------


#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/api/impl/apollo/apollo_impl.hpp"
#include "nic/apollo/api/impl/apollo/tep_impl_state.hpp"
#include "nic/apollo/api/impl/apollo/apollo_impl.hpp"
#include "gen/p4gen/apollo/include/p4pd.h"
#include "include/sdk/table.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_TEP_IMPL_STATE - TEP database functionality
/// \ingroup PDS_TEP
/// \@{

tep_impl_state::tep_impl_state(pds_state *state) {
    sdk::table::sdk_table_factory_params_t params = { 0 };
    sdk::table::sdk_table_api_params_t api_params;

    params.table_id = P4TBL_ID_TEP;
    params.entry_trace_en = true;
    tep_tbl_ = sldirectmap::factory(&params);
    SDK_ASSERT(tep_tbl_ != NULL);
    // reserve entry for mytep
    PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(&api_params,
                                          PDS_IMPL_MYTEP_HW_ID, NULL, NULL);
    SDK_ASSERT(tep_tbl_->reserve_index(&api_params) == SDK_RET_OK);
}

tep_impl_state::~tep_impl_state() {
    sldirectmap::destroy(tep_tbl_);
}

sdk_ret_t
tep_impl_state::table_transaction_begin(void) {
    //tep_tbl_->txn_start();
    return SDK_RET_OK;
}

sdk_ret_t
tep_impl_state::table_transaction_end(void) {
    //tep_tbl_->txn_end();
    return SDK_RET_OK;
}

/// \@}

}    // namespace impl
}    // namespace api
