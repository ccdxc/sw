//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// DHCP state handling
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/api/dhcp_state.hpp"

using namespace sdk;

namespace api {

dhcp_state::dhcp_state() {
    dhcp_policy_ht_ =
        ht::factory(PDS_MAX_DHCP_POLICY >> 2,
                    dhcp_policy::dhcp_policy_key_func_get,
                    sizeof(pds_obj_key_t));
    SDK_ASSERT(dhcp_policy_ht_ != NULL);
    dhcp_policy_slab_ = slab::factory("dhcp-policy", PDS_SLAB_ID_DHCP_POLICY,
                                     sizeof(dhcp_policy), 16, true, true, NULL);
    SDK_ASSERT(dhcp_policy_slab_ != NULL);
}

dhcp_state::~dhcp_state() {
    ht::destroy(dhcp_policy_ht_);
    slab::destroy(dhcp_policy_slab_);
}

dhcp_policy *
dhcp_state::alloc(void) {
    return ((dhcp_policy *)dhcp_policy_slab_->alloc());
}

sdk_ret_t
dhcp_state::insert(dhcp_policy *policy) {
    return dhcp_policy_ht_->insert_with_key(&policy->key_, policy,
                                            &policy->ht_ctxt_);
}

dhcp_policy *
dhcp_state::remove(dhcp_policy *policy) {
    return (dhcp_policy *)(dhcp_policy_ht_->remove(&policy->key_));
}

void
dhcp_state::free(dhcp_policy *policy) {
    dhcp_policy_slab_->free(policy);
}

dhcp_policy *
dhcp_state::find(pds_obj_key_t *key) const {
    return (dhcp_policy *)(dhcp_policy_ht_->lookup(key));
}

sdk_ret_t
dhcp_state::walk(state_walk_cb_t walk_cb, void *ctxt) {
    return dhcp_policy_ht_->walk(walk_cb, ctxt);
}

sdk_ret_t
dhcp_state::slab_walk(state_walk_cb_t walk_cb, void *ctxt) {
    walk_cb(dhcp_policy_slab_, ctxt);
    return SDK_RET_OK;
}

}    // namespace api
