//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// service mapping datapath database handling
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/lib/table/memhash/mem_hash.hpp"
#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/apollo/api/impl/apulu/pds_impl_state.hpp"
#include "nic/apollo/api/impl/apulu/service_impl.hpp"
#include "nic/apollo/api/include/pds_debug.hpp"
#include "gen/p4gen/apulu/include/p4pd.h"

using sdk::table::sdk_table_factory_params_t;

namespace api {
namespace impl {

/// \defgroup PDS_SERVICE_IMPL_STATE - service mapping database functionality
/// \ingroup PDS_SERVICE
/// \@{

svc_mapping_impl_state::svc_mapping_impl_state(pds_state *state) {
    sdk_table_factory_params_t tparams;

    // create a slab for mapping impl entries
    svc_mapping_impl_slab_ = slab::factory("svc-mapping-impl",
                                           PDS_SLAB_ID_SVC_MAPPING_IMPL,
                                           sizeof(svc_mapping_impl), 128,
                                           true, true, true, NULL);
    SDK_ASSERT(svc_mapping_impl_slab_!= NULL);

    // instantiate P4 svc mappping table instance for bookkeeping
    bzero(&tparams, sizeof(tparams));
    tparams.max_recircs = 8;
    tparams.entry_trace_en = false;
    tparams.table_id = P4TBL_ID_SERVICE_MAPPING;
    tparams.key2str = NULL;
    tparams.appdata2str = NULL;
    svc_mapping_tbl_ = slhash::factory(&tparams);
    SDK_ASSERT(svc_mapping_tbl_ != NULL);
}

svc_mapping_impl_state::~svc_mapping_impl_state() {
    slhash::destroy(svc_mapping_tbl_);
    slab::destroy(svc_mapping_impl_slab_);
}

svc_mapping_impl *
svc_mapping_impl_state::alloc(void) {
    return ((svc_mapping_impl *)svc_mapping_impl_slab_->alloc());
}

void
svc_mapping_impl_state::free(svc_mapping_impl *impl) {
    svc_mapping_impl_slab_->free(impl);
}

sdk_ret_t
svc_mapping_impl_state::table_transaction_begin(void) {
    svc_mapping_tbl_->txn_start();
    return SDK_RET_OK;
}

sdk_ret_t
svc_mapping_impl_state::table_transaction_end(void) {
    svc_mapping_tbl_->txn_end();
    return SDK_RET_OK;
}

sdk_ret_t
svc_mapping_impl_state::table_stats(debug::table_stats_get_cb_t cb,
                                    void *ctxt) {
    pds_table_stats_t stats;
    p4pd_table_properties_t tinfo;

    memset(&stats, 0, sizeof(pds_table_stats_t));
    p4pd_table_properties_get(P4TBL_ID_SERVICE_MAPPING, &tinfo);
    stats.table_name = tinfo.tablename;
    svc_mapping_tbl_->stats_get(&stats.api_stats, &stats.table_stats);
    cb(&stats, ctxt);
    return SDK_RET_OK;
}

sdk_ret_t
svc_mapping_impl_state::slab_walk(state_walk_cb_t walk_cb, void *ctxt) {
    walk_cb(svc_mapping_impl_slab_, ctxt);
    return SDK_RET_OK;
}

/// \@}

}    // namespace impl
}    // namespace api
