//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// vpc datapath database handling
///
//----------------------------------------------------------------------------

#include "nic/apollo/api/impl/apulu/vpc_impl_state.hpp"
#include "gen/p4gen/apulu/include/p4pd.h"
#include "nic/apollo/core/trace.hpp"

using sdk::table::sdk_table_factory_params_t;

namespace api {
namespace impl {

/// \defgroup PDS_VPC_IMPL_STATE - vpc database functionality
/// \ingroup PDS_VPC
/// @{

vpc_impl_state::vpc_impl_state(pds_state *state) {
    sdk_table_factory_params_t    tparams;

    // instantiate P4 tables for bookkeeping
    bzero(&tparams, sizeof(tparams));
    tparams.entry_trace_en = true;
    tparams.table_id = P4TBL_ID_VNI;
    vni_tbl_ = slhash::factory(&tparams);
    SDK_ASSERT(vni_tbl_ != NULL);
}

vpc_impl_state::~vpc_impl_state() {
    slhash::destroy(vni_tbl_);
}

sdk_ret_t
vpc_impl_state::table_transaction_begin(void) {
    vni_tbl_->txn_start();
    return SDK_RET_OK;
}

sdk_ret_t
vpc_impl_state::table_transaction_end(void) {
    vni_tbl_->txn_end();
    return SDK_RET_OK;
}

sdk_ret_t
vpc_impl_state::table_stats(debug::table_stats_get_cb_t cb, void *ctxt) {
    pds_table_stats_t stats;
    p4pd_table_properties_t tinfo;

    memset(&stats, 0, sizeof(pds_table_stats_t));
    p4pd_table_properties_get(P4TBL_ID_VNI_OTCAM, &tinfo);
    stats.table_name = tinfo.tablename;
    vni_tbl_->stats_get(&stats.api_stats, &stats.table_stats);
    cb(&stats, ctxt);

    return SDK_RET_OK;
}

/// @}

}    // namespace impl
}    // namespace api
