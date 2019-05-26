//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// vpc peering datapath database handling
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/apollo/api/impl/artemis/pds_impl_state.hpp"
#include "nic/apollo/api/impl/artemis/vpc_peer_impl.hpp"
#include "nic/apollo/api/include/pds_debug.hpp"
#include "nic/apollo/p4/include/artemis_defines.h"
#include "gen/p4gen/artemis/include/p4pd.h"

using sdk::table::sdk_table_factory_params_t;

namespace api {
namespace impl {

/// \defgroup PDS_VPC_PEER_IMPL_STATE - mapping database functionality
/// \ingroup PDS_MAPPING
/// \@{

vpc_peer_impl_state::vpc_peer_impl_state(pds_state *state) {
#if 0
    sdk_table_factory_params_t    mhparams;

    // instantiate P4 tables for bookkeeping
    bzero(&mhparams, sizeof(mhparams));
    mhparams.max_recircs = 8;
    mhparams.entry_trace_en = true;
    mhparams.table_id = P4_TBL_ID_VPC_PEER;
    mhparams.num_hints = P4_VPC_PEER_NUM_HINTS_PER_ENTRY;
    mhparams.key2str = NULL;
    mhparams.appdata2str = NULL;
    vpc_peer_tbl_ = mem_hash::factory(&mhparams);
    SDK_ASSERT(vpc_peer_tbl_ != NULL);
#endif

    // create a slab for mapping impl entries
    vpc_peer_impl_slab_ =
        slab::factory("vpc-peer-impl", PDS_SLAB_ID_VPC_PEER_IMPL,
                      sizeof(vpc_peer_impl), 16, true, true);
    SDK_ASSERT(vpc_peer_impl_slab_!= NULL);
}

vpc_peer_impl_state::~vpc_peer_impl_state() {
    mem_hash::destroy(vpc_peer_tbl_);
    slab::destroy(vpc_peer_impl_slab_);
}

vpc_peer_impl *
vpc_peer_impl_state::alloc(void) {
    return ((vpc_peer_impl *)vpc_peer_impl_slab_->alloc());
}

void
vpc_peer_impl_state::free(vpc_peer_impl *impl) {
    vpc_peer_impl_slab_->free(impl);
}

sdk_ret_t
vpc_peer_impl_state::table_transaction_begin(void) {
    vpc_peer_tbl_->txn_start();
    return SDK_RET_OK;
}

sdk_ret_t
vpc_peer_impl_state::table_transaction_end(void) {
    vpc_peer_tbl_->txn_end();
    return SDK_RET_OK;
}

sdk_ret_t
vpc_peer_impl_state::table_stats(debug::table_stats_get_cb_t cb, void *ctxt) {
    pds_table_stats_t stats;
    p4pd_table_properties_t tinfo;

#if 0
    memset(&stats, 0, sizeof(pds_table_stats_t));
    p4pd_global_table_properties_get(P4_TBL_ID_VPC_PEER, &tinfo);
    stats.table_name = tinfo.tablename;
    vpc_peer_tbl_->stats_get(&stats.api_stats, &stats.table_stats);
    cb(&stats, ctxt);
#endif

    return SDK_RET_OK;
}

/// \@}

}    // namespace impl
}    // namespace api
