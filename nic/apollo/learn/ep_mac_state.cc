//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// MAC Learning state handling
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/learn/ep_mac_state.hpp"

namespace learn {

ep_mac_state::ep_mac_state() {
    ep_mac_ht_ =
        ht::factory(EP_MAX_MAC_ENTRY >> 1, ep_mac_entry::ep_mac_key_func_get,
                    sizeof(ep_mac_key_t));
    SDK_ASSERT(ep_mac_ht_ != NULL);
    ep_mac_slab_ = slab::factory("ep_mac", LEARN_SLAB_ID_EP_MAC,
                                sizeof(ep_mac_entry), 16, true);
    SDK_ASSERT(ep_mac_slab_ != NULL);
}

ep_mac_state::~ep_mac_state() {
    ht::destroy(ep_mac_ht_);
    slab::destroy(ep_mac_slab_);
}

ep_mac_entry *
ep_mac_state::alloc(void) {
    return ((ep_mac_entry *)ep_mac_slab_->alloc());
}

void
ep_mac_state::free(ep_mac_entry *mac) {
    ep_mac_slab_->free(mac);
}

sdk_ret_t
ep_mac_state::insert(ep_mac_entry *mac) {
    return ep_mac_ht_->insert_with_key(&mac->key_, mac,
                                       &mac->ht_ctxt_);
}

ep_mac_entry *
ep_mac_state::remove(ep_mac_entry *mac) {
    return (ep_mac_entry *)(ep_mac_ht_->remove(&mac->key_));
}

const ep_mac_entry *
ep_mac_state::find(ep_mac_key_t *key) const {
    return (ep_mac_entry *)(ep_mac_ht_->lookup(key));
}

sdk_ret_t
ep_mac_state::walk(state_walk_cb_t walk_cb, void *user_data) {
    return ep_mac_ht_->walk(walk_cb, user_data);
}

sdk_ret_t
ep_mac_state::slab_walk(state_walk_cb_t walk_cb, void *user_data) {
    walk_cb(ep_mac_slab_, user_data);
    return SDK_RET_OK;
}

}    // namespace learn
