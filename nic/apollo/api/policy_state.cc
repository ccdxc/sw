//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// security policy database handling
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/policy_state.hpp"

namespace api {

/// \defgroup PDS_POLICY_STATE - security policy database functionality
/// \ingroup PDS_POLICY
/// \@{

policy_state::policy_state(sdk::lib::kvstore *kvs) {
    kvstore_ = kvs;
    policy_ht_ = ht::factory(PDS_MAX_SECURITY_POLICY >> 2,
                             policy::policy_key_func_get,
                             sizeof(pds_obj_key_t));
    SDK_ASSERT(policy_ht_ != NULL);

    policy_slab_ = slab::factory("security-policy", PDS_SLAB_ID_POLICY,
                                 sizeof(policy), 16, true, true, true, NULL);
    SDK_ASSERT(policy_slab_ != NULL);

    security_profile_ht_ =
        ht::factory(PDS_MAX_SECURITY_POLICY >> 2,
                    security_profile::security_profile_key_func_get,
                    sizeof(pds_obj_key_t));
    SDK_ASSERT(security_profile_ht_ != NULL);

    security_profile_slab_ = slab::factory("security-profile",
                                           PDS_SLAB_ID_SECURITY_PROFILE,
                                           sizeof(security_profile),
                                           4, true, true, true, NULL);
    SDK_ASSERT(security_profile_slab_ != NULL);
}

policy_state::~policy_state() {
    ht::destroy(policy_ht_);
    slab::destroy(policy_slab_);
    ht::destroy(security_profile_ht_);
    slab::destroy(security_profile_slab_);
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
policy_state::find_policy(pds_obj_key_t *policy_key) const {
    return (policy *)(policy_ht_->lookup(policy_key));
}

sdk_ret_t
policy_state::persist(policy *policy, pds_policy_spec_t *spec) {
    sdk_ret_t ret;

    if (policy->key_.valid()) {
        ret = kvstore_->insert(&policy->key_, sizeof(policy->key_),
                               spec->rule_info,
                               POLICY_RULE_SET_SIZE(spec->rule_info->num_rules));
        if (unlikely(ret != SDK_RET_OK)) {
            PDS_TRACE_ERR("Failed to persist policy %s in kvstore, err %u",
                          spec->key.str(), ret);
        }
        return ret;
    }
    return SDK_RET_OK;
}

sdk_ret_t
policy_state::perish(const pds_obj_key_t& key) {
    sdk_ret_t ret;

    if (key.valid()) {
        ret = kvstore_->remove(&key, sizeof(key));
        if (unlikely(ret != SDK_RET_OK)) {
            PDS_TRACE_ERR("Failed to remove policy %s from kvstore, err %u",
                          key.str(), ret);
        }
        return ret;
    }
    return SDK_RET_OK;
}

sdk_ret_t
policy_state::walk(state_walk_cb_t walk_cb, void *ctxt) {
    return policy_ht_->walk(walk_cb, ctxt);
}

security_profile *
policy_state::alloc_security_profile(void) {
    return ((security_profile *)security_profile_slab_->alloc());
}

sdk_ret_t
policy_state::insert(security_profile *obj) {
    return security_profile_ht_->insert_with_key(&obj->key_, obj,
                                                 &obj->ht_ctxt_);
}

security_profile *
policy_state::remove(security_profile *obj) {
    return (security_profile *)(security_profile_ht_->remove(&obj->key_));
}

void
policy_state::free(security_profile *profile) {
    security_profile_slab_->free(profile);
}

security_profile *
policy_state::find_security_profile(pds_obj_key_t *key) const {
    return (security_profile *)(security_profile_ht_->lookup(key));
}

sdk_ret_t
policy_state::slab_walk(state_walk_cb_t walk_cb, void *ctxt) {
    walk_cb(policy_slab_, ctxt);
    walk_cb(security_profile_slab_, ctxt);
    return SDK_RET_OK;
}

/// \@}    // end of PDS_VPC_STATE

}    // namespace api
