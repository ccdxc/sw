//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// subnet state handling
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/api/subnet_state.hpp"

using namespace sdk;

namespace api {

subnet_state::subnet_state() {
    subnet_ht_ =
        ht::factory(PDS_MAX_SUBNET >> 1, subnet_entry::subnet_key_func_get,
                    sizeof(pds_obj_key_t));
    SDK_ASSERT(subnet_ht_ != NULL);
    // we should accomodate one extra subnet of type provider/underlay/internet
    subnet_idxr_ = indexer::factory(PDS_MAX_SUBNET + 1);
    SDK_ASSERT(subnet_idxr() != NULL);
    subnet_slab_ = slab::factory("subnet", PDS_SLAB_ID_SUBNET,
                                 sizeof(subnet_entry), 16, true, true, NULL);
    SDK_ASSERT(subnet_slab_ != NULL);
}

subnet_state::~subnet_state() {
    ht::destroy(subnet_ht_);
    indexer::destroy(subnet_idxr());
    slab::destroy(subnet_slab_);
}

subnet_entry *
subnet_state::alloc(void) {
    return ((subnet_entry *)subnet_slab_->alloc());
}

sdk_ret_t
subnet_state::insert(subnet_entry *subnet) {
    return subnet_ht_->insert_with_key(&subnet->key_, subnet,
                                        &subnet->ht_ctxt_);
}

subnet_entry *
subnet_state::remove(subnet_entry *subnet) {
    return (subnet_entry *)(subnet_ht_->remove(&subnet->key_));
}

void
subnet_state::free(subnet_entry *subnet) {
    subnet_slab_->free(subnet);
}

subnet_entry *
subnet_state::find(pds_obj_key_t *subnet_key) const {
    return (subnet_entry *)(subnet_ht_->lookup(subnet_key));
}

sdk_ret_t
subnet_state::slab_walk(state_walk_cb_t walk_cb, void *ctxt) {
    walk_cb(subnet_slab_, ctxt);
    return SDK_RET_OK;
}

}    // namespace api
