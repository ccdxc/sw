//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// VNIC datapath database handling
///
//----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_vnic.hpp"
#include "nic/apollo/api/impl/apulu/pds_impl_state.hpp"
#include "nic/apollo/api/impl/apulu/vnic_impl.hpp"
#include "nic/apollo/api/impl/apulu/vnic_impl_state.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_VNIC_IMPL_STATE - VNIC database functionality
/// \ingroup PDS_VNIC
/// @{

vnic_impl_state::vnic_impl_state(pds_state *state) {
    p4pd_table_properties_t tinfo;
    sdk_table_factory_params_t table_params;

    // allocate indexer for vnic hw id allocation and reserve 0th entry
    p4pd_global_table_properties_get(P4TBL_ID_VNIC_TX_STATS, &tinfo);
    vnic_idxr_ = rte_indexer::factory(tinfo.tabledepth, true, true);
    SDK_ASSERT(vnic_idxr_ != NULL);

    // create a slab for vnic impl entries
    vnic_impl_slab_ =
        slab::factory("vnic-impl", PDS_SLAB_ID_VNIC_IMPL,
                      sizeof(vnic_impl), 16, true, true, true, NULL);
    SDK_ASSERT(vnic_impl_slab_ != NULL);

    // create ht for vnic id to key mapping
    impl_ht_ = ht::factory(PDS_MAX_VNIC >> 2,
                           vnic_impl::key_get,
                           sizeof(uint16_t));
    SDK_ASSERT(impl_ht_ != NULL);
}

vnic_impl_state::~vnic_impl_state() {
    rte_indexer::destroy(vnic_idxr_);
    slab::destroy(vnic_impl_slab_);
    ht::destroy(impl_ht_);
}

vnic_impl *
vnic_impl_state::alloc(void) {
    return (vnic_impl *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_VNIC_IMPL,
                                   sizeof(vnic_impl));
}

void
vnic_impl_state::free(vnic_impl *impl) {
    SDK_FREE(SDK_MEM_ALLOC_PDS_VNIC_IMPL, impl);
}

sdk_ret_t
vnic_impl_state::table_transaction_begin(void) {
    return SDK_RET_OK;
}

sdk_ret_t
vnic_impl_state::table_transaction_end(void) {
    return SDK_RET_OK;
}

sdk_ret_t
vnic_impl_state::slab_walk(state_walk_cb_t walk_cb, void *ctxt) {
    walk_cb(vnic_impl_slab_, ctxt);
    return SDK_RET_OK;
}

vnic_impl *
vnic_impl_state::find(uint16_t hw_id) {
    return (vnic_impl *)impl_ht_->lookup(&hw_id);
}

sdk_ret_t
vnic_impl_state::insert(uint16_t hw_id, vnic_impl *impl) {
    impl_ht_->insert_with_key(&hw_id, impl, impl->ht_ctxt());
    return SDK_RET_OK;
}

sdk_ret_t
vnic_impl_state::update(uint16_t hw_id, vnic_impl *impl) {
    impl_ht_->remove(&hw_id);
    impl_ht_->insert_with_key(&hw_id, impl, impl->ht_ctxt());
    return SDK_RET_OK;
}

sdk_ret_t
vnic_impl_state::remove(uint16_t hw_id) {
    vnic_impl *vnic;

    vnic = (vnic_impl *)impl_ht_->remove(&hw_id);
    if (!vnic) {
        PDS_TRACE_ERR("Failed to find vnic impl for hw id %u", hw_id);
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    return SDK_RET_OK;
}

/// \@}

}    // namespace impl
}    // namespace api
