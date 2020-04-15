//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains Tunnel EndPoint (TEP) datapath database handling
///
//----------------------------------------------------------------------------


#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/api/impl/apulu/tep_impl.hpp"
#include "nic/apollo/api/impl/apulu/tep_impl_state.hpp"
#include "gen/p4gen/apulu/include/p4pd.h"

namespace api {
namespace impl {

/// \defgroup PDS_TEP_IMPL_STATE - TEP database functionality
/// \ingroup PDS_TEP
/// \@{

tep_impl_state::tep_impl_state(pds_state *state) {
    p4pd_table_properties_t tinfo;

    // instantiate TUNNEL table for bookkeeping
    p4pd_global_table_properties_get(P4TBL_ID_TUNNEL, &tinfo);
    tunnel_idxr_ = rte_indexer::factory(tinfo.tabledepth, true, true);
    SDK_ASSERT(tunnel_idxr_ != NULL);

    // instantiate TUNNEL2 table for bookkeeping (0th entry is reserved)
    p4pd_global_table_properties_get(P4TBL_ID_TUNNEL2, &tinfo);
    tunnel2_idxr_ = rte_indexer::factory(tinfo.tabledepth, true, true);
    SDK_ASSERT(tunnel2_idxr_ != NULL);
}

tep_impl_state::~tep_impl_state() {
    rte_indexer::destroy(tunnel_idxr_);
    rte_indexer::destroy(tunnel2_idxr_);
}

tep_impl *
tep_impl_state::alloc(void) {
    return (tep_impl *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_TEP_IMPL, sizeof(tep_impl));
}

void
tep_impl_state::free(tep_impl *impl) {
    SDK_FREE(SDK_MEM_ALLOC_PDS_TEP_IMPL, impl);
}

sdk_ret_t
tep_impl_state::table_transaction_begin(void) {
    return SDK_RET_OK;
}

sdk_ret_t
tep_impl_state::table_transaction_end(void) {
    return SDK_RET_OK;
}

/// \@}

}    // namespace impl
}    // namespace api
