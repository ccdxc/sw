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
#include "nic/apollo/api/impl/apollo/tep_impl_state.hpp"
#include "gen/p4gen/apollo/include/p4pd.h"

namespace api {
namespace impl {

/// \defgroup PDS_TEP_IMPL_STATE - TEP database functionality
/// \ingroup PDS_TEP
/// \@{

tep_impl_state::tep_impl_state(pds_state *state) {
    p4pd_table_properties_t tinfo;

    // instantiate P4 tables for bookkeeping
    p4pd_table_properties_get(P4TBL_ID_TEP, &tinfo);
    // TODO: table_health_monitor_cb is passed as NULL here !!
    tep_tbl_ = directmap::factory(tinfo.tablename, P4TBL_ID_TEP,
                                  tinfo.tabledepth,
                                  tinfo.actiondata_struct_size,
                                  false, true, NULL);
    SDK_ASSERT(tep_tbl_ != NULL);
}

tep_impl_state::~tep_impl_state() {
    directmap::destroy(tep_tbl_);
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
