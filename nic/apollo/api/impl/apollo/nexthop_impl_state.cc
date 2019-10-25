//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// nexthop datapath database handling
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/apollo/api/include/pds_nexthop.hpp"
#include "nic/apollo/api/impl/apollo/apollo_impl.hpp"
#include "nic/apollo/api/impl/apollo/nexthop_impl_state.hpp"
#include "gen/p4gen/apollo/include/p4pd.h"

namespace api {
namespace impl {

/// \defgroup PDS_NEXTHOP_IMPL_STATE - nexthop database functionality
/// \ingroup PDS_NEXTHOP
/// \@{

nexthop_impl_state::nexthop_impl_state(pds_state *state) {
    sdk::table::sdk_table_factory_params_t params = { 0 };
    sdk::table::sdk_table_api_params_t api_params;

    params.table_id = P4TBL_ID_NEXTHOP;
    params.entry_trace_en = true;
    nh_tbl_ = sldirectmap::factory(&params);
    SDK_ASSERT(nh_tbl_ != NULL);

    // reserve system default blackhole/drop nexthop entry
    PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(&api_params,
                                          PDS_IMPL_SYSTEM_DROP_NEXTHOP_HW_ID,
                                          NULL, NULL);
    SDK_ASSERT(nh_tbl_->reserve_index(&api_params) == SDK_RET_OK);

    // reserve mytep nexthop entry
    PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(&api_params,
                                          PDS_IMPL_MYTEP_NEXTHOP_HW_ID,
                                          NULL, NULL);
    SDK_ASSERT(nh_tbl_->reserve_index(&api_params) == SDK_RET_OK);
}

nexthop_impl_state::~nexthop_impl_state() {
    sldirectmap::destroy(nh_tbl_);
}

sdk_ret_t
nexthop_impl_state::table_transaction_begin(void) {
    //nh_tbl_->txn_start();
    return SDK_RET_OK;
}

sdk_ret_t
nexthop_impl_state::table_transaction_end(void) {
    //nh_tbl_->txn_end();
    return SDK_RET_OK;
}

/// \@}    // end of PDS_NEXTHOP_IMPL_STATE

}    // namespace impl
}    // namespace api
