//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// apulu pipeline global state maintenance
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/table.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/api/impl/apulu/apulu_impl_state.hpp"
#include "gen/p4gen/apulu/include/p4pd.h"

using sdk::table::sdk_table_factory_params_t;

namespace api {
namespace impl {

/// \defgroup PDS_APULU_IMPL_STATE - apulu database functionality
/// \ingroup PDS_APULU
/// \@{

apulu_impl_state::apulu_impl_state(pds_state *state) {
    p4pd_table_properties_t tinfo;
    sdk_table_factory_params_t table_params;

    memset(&table_params, 0, sizeof(table_params));
    table_params.table_id = P4TBL_ID_P4I_DROP_STATS;
    table_params.entry_trace_en = false;
    ingress_drop_stats_tbl_ = sltcam::factory(&table_params);
    SDK_ASSERT(ingress_drop_stats_tbl() != NULL);

    memset(&table_params, 0, sizeof(table_params));
    table_params.table_id = P4TBL_ID_P4E_DROP_STATS;
    table_params.entry_trace_en = false;
    egress_drop_stats_tbl_ = sltcam::factory(&table_params);
    SDK_ASSERT(egress_drop_stats_tbl() != NULL);

    // allocate indexer for NACL table
    p4pd_global_table_properties_get(P4TBL_ID_NACL, &tinfo);
    nacl_idxr_ = rte_indexer::factory(tinfo.tabledepth, true, false);
    SDK_ASSERT(nacl_idxr_ != NULL);

    // bookkeeping for CoPP table
    p4pd_global_table_properties_get(P4TBL_ID_COPP, &tinfo);
    copp_idxr_ = rte_indexer::factory(tinfo.tabledepth, true, true);
    SDK_ASSERT(copp_idxr_ != NULL);

    // NAT table bookkeeping (reserve 0th entry for no xlation)
    p4pd_table_properties_get(P4TBL_ID_NAT, &tinfo);
    nat_idxr_ = rte_indexer::factory(tinfo.tabledepth, true, true);
    SDK_ASSERT(nat_idxr_ != NULL);

}

apulu_impl_state::~apulu_impl_state() {
    sltcam::destroy(ingress_drop_stats_tbl_);
    sltcam::destroy(egress_drop_stats_tbl_);
    rte_indexer::destroy(nacl_idxr_);
    rte_indexer::destroy(copp_idxr_);
    rte_indexer::destroy(nat_idxr_);
}

sdk_ret_t
apulu_impl_state::table_stats(debug::table_stats_get_cb_t cb, void *ctxt) {
    return SDK_RET_OK;
}

/// \@}

}    // namespace impl
}    // namespace api
