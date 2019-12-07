/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    policy_state.cc
 *
 * @brief   security policy database handling
 */

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/api/policy_state.hpp"

namespace api {

/**
 * @defgroup PDS_POLICY_STATE - security policy database functionality
 * @ingroup PDS_POLICY
 * @{
 */

policy_state::policy_state() {
    // TODO: need to tune multi-threading related params later
    policy_ht_ = ht::factory(PDS_MAX_SECURITY_POLICY >> 2,
                             policy::policy_key_func_get,
                             sizeof(pds_policy_key_t));
    SDK_ASSERT(policy_ht_ != NULL);

    policy_slab_ = slab::factory("security-policy", PDS_SLAB_ID_POLICY,
                                 sizeof(policy), 16, true, true, true, NULL);
    SDK_ASSERT(policy_slab_ != NULL);
}

policy_state::~policy_state() {
    ht::destroy(policy_ht_);
    slab::destroy(policy_slab_);
}

policy *
policy_state::alloc(void) {
    return ((policy *)policy_slab_->alloc());
}

sdk_ret_t
policy_state::insert(policy *obj) {
    return policy_ht_->insert_with_key(&obj->key_, obj, &obj->ht_ctxt_);
}

policy *
policy_state::remove(policy *obj) {
    return (policy *)(policy_ht_->remove(&obj->key_));
}

void
policy_state::free(policy *policy) {
    policy_slab_->free(policy);
}

policy *
policy_state::find(pds_policy_key_t *policy_key) const {
    return (policy *)(policy_ht_->lookup(policy_key));
}

sdk_ret_t
policy_state::slab_walk(state_walk_cb_t walk_cb, void *ctxt) {
    walk_cb(policy_slab_, ctxt);
    return SDK_RET_OK;
}

/** @} */    // end of PDS_POLICY_STATE

}    // namespace api
