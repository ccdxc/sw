//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// IP Learning state handling
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/learn/ep_ip_state.hpp"

namespace learn {

ep_ip_state::ep_ip_state() {
    ep_ip_ht_ =
        ht::factory(EP_MAX_IP_ENTRY >> 1, ep_ip_entry::ep_ip_key_func_get,
                    sizeof(ep_ip_key_t));
    SDK_ASSERT(ep_ip_ht_ != NULL);
    ep_ip_slab_ = slab::factory("ep_ip", LEARN_SLAB_ID_EP_IP,
                                sizeof(ep_ip_entry), 16, true);
    SDK_ASSERT(ep_ip_slab_ != NULL);
}

ep_ip_state::~ep_ip_state() {
    ht::destroy(ep_ip_ht_);
    slab::destroy(ep_ip_slab_);
}

ep_ip_entry *
ep_ip_state::alloc(void) {
    return ((ep_ip_entry *)ep_ip_slab_->alloc());
}

void
ep_ip_state::free(ep_ip_entry *ip) {
    ep_ip_slab_->free(ip);
}

sdk_ret_t
ep_ip_state::insert(ep_ip_entry *ip) {
    return ep_ip_ht_->insert_with_key(&ip->key_, ip,
                                      &ip->ht_ctxt_);
}

ep_ip_entry *
ep_ip_state::remove(ep_ip_entry *ip) {
    return (ep_ip_entry *)(ep_ip_ht_->remove(&ip->key_));
}

const ep_ip_entry *
ep_ip_state::find(ep_ip_key_t *key) const {
    return (ep_ip_entry *)(ep_ip_ht_->lookup(key));
}

sdk_ret_t
ep_ip_state::walk(state_walk_cb_t walk_cb, void *user_data) {
	return ep_ip_ht_->walk(walk_cb, user_data);
}

sdk_ret_t
ep_ip_state::slab_walk(state_walk_cb_t walk_cb, void *user_data) {
    walk_cb(ep_ip_slab_, user_data);
    return SDK_RET_OK;
}

}    // namespace learn
