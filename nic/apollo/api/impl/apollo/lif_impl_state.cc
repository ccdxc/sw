//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// lif implementation state maintenance
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/p4/p4_api.hpp"
#include "gen/p4gen/apollo_txdma/include/apollo_txdma_p4pd.h"
#include "nic/apollo/api/impl/lif_impl_state.hpp"

/// \defgroup PDS_LIF_IMPL_STATE - lif state functionality
/// \ingroup PDS_LIF
/// \@{

#define PDS_MAX_LIFS    512

namespace api {
namespace impl {

lif_impl_state::lif_impl_state(pds_state *state) {
    p4pd_table_properties_t    tinfo;

    // maintain all lifs in the system in a hash table
    lif_ht_ = ht::factory(PDS_MAX_LIFS >> 2,
                          lif_impl::lif_key_func_get,
                          lif_impl::lif_hash_func_compute,
                          lif_impl::lif_key_func_compare);
    SDK_ASSERT(lif_ht_ != NULL);

    p4pluspd_txdma_table_properties_get(
                P4_APOLLO_TXDMA_TBL_ID_TX_TABLE_S5_T4_LIF_RATE_LIMITER_TABLE,
                &tinfo);
    tx_rate_limiter_tbl_ =
        directmap::factory(tinfo.tablename,
                           P4_APOLLO_TXDMA_TBL_ID_TX_TABLE_S5_T4_LIF_RATE_LIMITER_TABLE,
                           tinfo.tabledepth, tinfo.actiondata_struct_size,
                           false, true, NULL);
    SDK_ASSERT(tx_rate_limiter_tbl_ != NULL);
}

lif_impl_state::~lif_impl_state() {
    ht::destroy(lif_ht_);
    directmap::destroy(tx_rate_limiter_tbl_);
}

lif_impl *
lif_impl_state::alloc(void) {
    return (lif_impl *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_LIF_IMPL, sizeof(lif_impl));
}

sdk_ret_t
lif_impl_state::insert(lif_impl *impl) {
    return lif_ht_->insert_with_key(&impl->key_, impl, &impl->ht_ctxt_);
}

lif_impl *
lif_impl_state::remove(lif_impl *impl) {
    return (lif_impl *)(lif_ht_->remove(&impl->key_));
}

void
lif_impl_state::free(lif_impl *impl) {
    SDK_FREE(SDK_MEM_ALLOC_PDS_LIF_IMPL, impl);
}

lif_impl *
lif_impl_state::find(pds_lif_key_t *key) const {
    return (lif_impl *)(lif_ht_->lookup(key));
}

sdk_ret_t
lif_impl_state::walk(state_walk_cb_t walk_cb, void *ctxt) {
    return lif_ht_->walk(walk_cb, ctxt);
}

/// \@}

}    // namespace impl
}    // namespace api
