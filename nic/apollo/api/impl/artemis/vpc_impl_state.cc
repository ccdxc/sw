//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// vpc datapath database handling
///
//----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/apollo/api/impl/artemis/vpc_impl_state.hpp"
#include "gen/p4gen/artemis/include/p4pd.h"

namespace api {
namespace impl {

/// \defgroup PDS_VPC_IMPL_STATE - vpc database functionality
/// \ingroup PDS_VPC
/// @{

vpc_impl_state::vpc_impl_state(pds_state *state) {
    p4pd_table_properties_t       tinfo, oflow_tinfo;
    sdk_table_factory_params_t    table_params;

    // TEP1_RX tcam table
    memset(&table_params, 0, sizeof(table_params));
    table_params.table_id = P4TBL_ID_TEP1_RX;
    table_params.entry_trace_en = true;
    tep1_rx_tbl_ = sltcam::factory(&table_params);
    SDK_ASSERT(tep1_rx_tbl_ != NULL);
}

vpc_impl_state::~vpc_impl_state() {
    sltcam::destroy(tep1_rx_tbl_);
}

sdk_ret_t
vpc_impl_state::table_transaction_begin(void) {
    tep1_rx_tbl_->txn_start();
    return SDK_RET_OK;
}

sdk_ret_t
vpc_impl_state::table_transaction_end(void) {
    tep1_rx_tbl_->txn_end();
    return SDK_RET_OK;
}

/// @}

}    // namespace impl
}    // namespace api
