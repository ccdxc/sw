//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// subnet datapath database handling
///
//----------------------------------------------------------------------------

#include "nic/apollo/api/impl/apulu/subnet_impl.hpp"
#include "nic/apollo/api/impl/apulu/subnet_impl_state.hpp"
#include "gen/p4gen/apulu/include/p4pd.h"

namespace api {
namespace impl {

/// \defgroup PDS_SUBNET_IMPL_STATE - subnet database functionality
/// \ingroup PDS_SUBNET
/// @{

subnet_impl_state::subnet_impl_state(pds_state *state) {
    p4pd_table_properties_t    tinfo;

    p4pd_global_table_properties_get(P4TBL_ID_BD, &tinfo);
    // create indexer for subnet hw id allocation and reserve 0th entry
    subnet_idxr_ = rte_indexer::factory(tinfo.tabledepth, true, true);
    SDK_ASSERT(subnet_idxr_ != NULL);

    impl_ht_ = ht::factory(PDS_MAX_SUBNET >> 2,
                           subnet_impl::key_get,
                           sizeof(uint16_t));
    SDK_ASSERT(impl_ht_ != NULL);
}

subnet_impl_state::~subnet_impl_state() {
    rte_indexer::destroy(subnet_idxr_);
    ht::destroy(impl_ht_);
}

subnet_impl *
subnet_impl_state::alloc(void) {
    return (subnet_impl *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_SUBNET_IMPL,
                                     sizeof(subnet_impl));
}

void
subnet_impl_state::free(subnet_impl *impl) {
    SDK_FREE(SDK_MEM_ALLOC_PDS_SUBNET_IMPL, impl);
}

sdk_ret_t
subnet_impl_state::table_transaction_begin(void) {
    return SDK_RET_OK;
}

sdk_ret_t
subnet_impl_state::table_transaction_end(void) {
    return SDK_RET_OK;
}

sdk_ret_t
subnet_impl_state::table_stats(debug::table_stats_get_cb_t cb, void *ctxt) {
    return SDK_RET_OK;
}

subnet_impl *
subnet_impl_state::find(uint16_t hw_id) {
    return (subnet_impl *)impl_ht_->lookup(&hw_id);
}

sdk_ret_t
subnet_impl_state::insert(uint16_t hw_id, subnet_impl *impl) {
    impl_ht_->insert_with_key(&hw_id, impl, impl->ht_ctxt());
    return SDK_RET_OK;
}

sdk_ret_t
subnet_impl_state::update(uint16_t hw_id, subnet_impl *impl) {
    if (impl_ht_->remove(&hw_id)) {
        return impl_ht_->insert_with_key(&hw_id, impl, impl->ht_ctxt());
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
subnet_impl_state::remove(uint16_t hw_id) {
    subnet_impl *subnet = NULL;

    subnet = (subnet_impl *) impl_ht_->remove(&hw_id);
    if (!subnet) {
        PDS_TRACE_ERR("subnet impl not found for hw id %u", hw_id);
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    return SDK_RET_OK;
}

/// @}

}    // namespace impl
}    // namespace api
