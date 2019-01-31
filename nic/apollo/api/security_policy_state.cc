/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    security_policy_state.cc
 *
 * @brief   security policy database handling
 */

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/api/security_policy_state.hpp"

namespace api {

/**
 * @defgroup OCI_SECURITY_POLICY_STATE - security policy database functionality
 * @ingroup OCI_SECURITY_POLICY
 * @{
 */

/**
 * @brief    constructor
 */
security_policy_state::security_policy_state() {
    // TODO: need to tune multi-threading related params later
    security_policy_ht_ =
        ht::factory(OCI_MAX_SECURITY_POLICY >> 2,
                    security_policy::security_policy_key_func_get,
                    security_policy::security_policy_hash_func_compute,
                    security_policy::security_policy_key_func_compare);
    SDK_ASSERT(security_policy_ht_ != NULL);

    security_policy_slab_ =
        slab::factory("security-policy", OCI_SLAB_ID_SECURITY_POLICY,
                      sizeof(security_policy), 16, true, true, true, NULL);
    SDK_ASSERT(security_policy_slab_ != NULL);
}

/**
 * @brief    destructor
 */
security_policy_state::~security_policy_state() {
    ht::destroy(security_policy_ht_);
    slab::destroy(security_policy_slab_);
}

/**
 * @brief     allocate security policy instance
 * @return    pointer to the allocated security policy, NULL if no memory
 */
security_policy *
security_policy_state::security_policy_alloc(void) {
    return ((security_policy *)security_policy_slab_->alloc());
}

/**
 * @brief      free security policy instance back to slab
 * @param[in]  rtrable pointer to the allocated security policy instance
 */
void
security_policy_state::security_policy_free(security_policy *policy) {
    security_policy_slab_->free(policy);
}

/**
 * @brief        lookup security policy in database with given key
 * @param[in]    policy_key security policy key
 * @return       pointer to the security policy instance found or NULL
 */
security_policy *
security_policy_state::security_policy_find(oci_security_policy_key_t *policy_key) const {
    return (security_policy *)(security_policy_ht_->lookup(policy_key));
}

/** @} */    // end of OCI_SECURITY_POLICY_STATE

}    // namespace api
