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
                           vnic_entry::vnic_hash_func_compute,
                           vnic_entry::vnic_key_func_compare);
    SDK_ASSERT(vnic_ht() != NULL);

    vnic_slab_ = slab::factory("vnic", PDS_SLAB_ID_VNIC, sizeof(vnic_entry),
                               16, true, true, true, NULL);
    SDK_ASSERT(vnic_slab() != NULL);
}

vnic_state::~vnic_state() {
    ht::destroy(vnic_ht());
    slab::destroy(vnic_slab());
}

vnic_entry *
vnic_state::alloc(void) {
    return ((vnic_entry *)vnic_slab()->alloc());
}

sdk_ret_t
vnic_state::insert(vnic_entry *vnic) {
    return vnic_ht()->insert_with_key(&vnic->key_, vnic,
                                      &vnic->ht_ctxt_);
}

vnic_entry *
vnic_state::remove(vnic_entry *vnic) {
    return (vnic_entry *)(vnic_ht()->remove(&vnic->key_));
}

void
vnic_state::free(vnic_entry *vnic) {
    vnic_slab()->free(vnic);
}

vnic_entry *
vnic_state::find(pds_vnic_key_t *vnic_key) const {
    return (vnic_entry *)(vnic_ht()->lookup(vnic_key));
}

sdk_ret_t
vnic_state::walk(state_walk_cb_t walk_cb, void *ctxt) {
    return vnic_ht()->walk(walk_cb, ctxt);
}

/// \@}

}    // namespace api
