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

/**
 * @brief    constructor
 */
policy_state::policy_state() {
    // TODO: need to tune multi-threading related params later
    policy_ht_ = ht::factory(PDS_MAX_SECURITY_POLICY >> 2,
                             policy::policy_key_func_get,
                             policy::key_size());
    SDK_ASSERT(policy_ht_ != NULL);

    policy_slab_ = slab::factory("security-policy", PDS_SLAB_ID_POLICY,
                                 sizeof(policy), 16, true, true, true, NULL);
    SDK_ASSERT(policy_slab_ != NULL);
}

/**
 * @brief    destructor
 */
policy_state::~policy_state() {
    ht::destroy(policy_ht_);
    slab::destroy(policy_slab_);
}

/**
 * @brief     allocate security policy instance
 * @return    pointer to the allocated security policy, NULL if no memory
 */
policy *
policy_state::alloc(void) {
    return ((policy *)policy_slab_->alloc());
}

/**
 * @brief      free security policy instance back to slab
 * @param[in]  rtrable pointer to the allocated security policy instance
 */
void
policy_state::free(policy *policy) {
    policy_slab_->free(policy);
}

/**
 * @brief        lookup security policy in database with given key
 * @param[in]    policy_key security policy key
 * @return       pointer to the security policy instance found or NULL
 */
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
