/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    vnic_state.cc
 *
 * @brief   vnic database handling
 */

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/api/vnic_state.hpp"

namespace api {

/// \defgroup PDS_VNIC_STATE - vnic database functionality
/// \ingroup PDS_VNIC
/// \@{

vnic_state::vnic_state() {
    // TODO: need to tune multi-threading related params later
    vnic_ht_ = ht::factory(PDS_MAX_VNIC >> 2,
                           vnic_entry::vnic_key_func_get,
                           vnic_entry::key_size());
    SDK_ASSERT(vnic_ht_ != NULL);

    vnic_slab_ = slab::factory("vnic", PDS_SLAB_ID_VNIC, sizeof(vnic_entry),
                               16, true, true, true, NULL);
    SDK_ASSERT(vnic_slab_ != NULL);
}

vnic_state::~vnic_state() {
    ht::destroy(vnic_ht_);
    slab::destroy(vnic_slab_);
}

vnic_entry *
vnic_state::alloc(void) {
    return ((vnic_entry *)vnic_slab_->alloc());
}

sdk_ret_t
vnic_state::insert(vnic_entry *vnic) {
    return vnic_ht_->insert_with_key(&vnic->key_, vnic,
                                      &vnic->ht_ctxt_);
}

vnic_entry *
vnic_state::remove(vnic_entry *vnic) {
    return (vnic_entry *)(vnic_ht_->remove(&vnic->key_));
}

void
vnic_state::free(vnic_entry *vnic) {
    vnic_slab_->free(vnic);
}

vnic_entry *
vnic_state::find(pds_vnic_key_t *vnic_key) const {
    return (vnic_entry *)(vnic_ht_->lookup(vnic_key));
}

sdk_ret_t
vnic_state::walk(state_walk_cb_t walk_cb, void *ctxt) {
    return vnic_ht_->walk(walk_cb, ctxt);
}

sdk_ret_t
vnic_state::slab_walk(state_walk_cb_t walk_cb, void *ctxt) {
    walk_cb(vnic_slab_, ctxt);
    return SDK_RET_OK;
}

/// \@}

}    // namespace api
