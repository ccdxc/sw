/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    policy_state.hpp
 *
 * @brief   policy database handling
 */

#ifndef __POLICY_STATE_HPP__
#define __POLICY_STATE_HPP__

#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/sdk/lib/kvstore/kvstore.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/policy.hpp"
#include "nic/apollo/api/security_profile.hpp"

namespace api {

/**
 * @defgroup PDS_POLICY_STATE - security policy state/db functionality
 * @ingroup PDS_POLICY
 * @{
 */

/**
 * @brief    state maintained for security policies
 */
class policy_state : public state_base {
public:
    /// \brief    constructor
    /// \param[in] kvs pointer to key-value store instance
    policy_state(sdk::lib::kvstore *kvs);

    /// \brief    destructor
    ~policy_state();

    /// \brief    allocate memory required for a security policy instance
    /// \return    pointer to the allocated security policy instance,
    ///            or NULL if no memory
    ///
    policy *alloc(void);

    /// \brief    insert given policy instance into the policy db
    /// \param[in] obj    policy to be added to the db
    /// \return   SDK_RET_OK on success, failure status code on error
    sdk_ret_t insert(policy *obj);

    /// \brief     remove the given instance of policy object from db
    /// \param[in] obj    policy entry to be deleted from the db
    /// \return    pointer to the removed policy instance or NULL,
    ///            if not found
    policy *remove(policy *obj);

    /// \brief      free security policy instance back to slab
    /// \param[in]  policy   pointer to the allocated security
    ///             policy instance
    void free(policy *policy);

    /// \brief     lookup a security policy in database given the key
    /// \param[in] policy_key security policy key
    policy *find_policy(pds_obj_key_t *policy_key) const;

    /// \brief      persist the given security policy
    /// \param[in]  policy policy object instance
    /// \param[in]  spec    policy configuration to be persisted
    /// \return     SDK_RET_OK on success, failure status code on error
    sdk_ret_t persist(policy *policy, pds_policy_spec_t *spec);

    /// \brief      destroy any persisted state of the given security policy
    /// \param[in]  key    key of the policy to be removed from persistent db
    /// \return     SDK_RET_OK on success, failure status code on error
    sdk_ret_t perish(const pds_obj_key_t& key);

    /// \brief    allocate memory required for a security profile instance
    /// \return    pointer to the allocated security profile instance,
    ///            or NULL if no memory
    security_profile *alloc_security_profile(void);

    /// \brief    insert given security profile instance into the
    ///           security profile db
    /// \param[in] profile    security profile to be added to the db
    /// \return   SDK_RET_OK on success, failure status code on error
    sdk_ret_t insert(security_profile *profile);

    /// \brief     remove the given instance of security profile object from db
    /// \param[in] profile security profile entry to be deleted from the db
    /// \return    pointer to the removed security profile instance or NULL,
    ///            if not found
    security_profile *remove(security_profile *profile);

    /// \brief API to walk all the db elements
    /// \param[in] walk_cb    callback to be invoked for every node
    /// \param[in] ctxt       opaque context passed back to the callback
    /// \return   SDK_RET_OK on success, failure status code on error
    sdk_ret_t walk(state_walk_cb_t walk_cb, void *ctxt) override;

    /// \brief      free security security profile instance back to slab
    /// \param[in]  profile   pointer to the allocated security profile instance
    void free(security_profile *profile);

    /// \brief     lookup a security security profile in database given the key
    /// \param[in] key    security profile key
    security_profile *find_security_profile(pds_obj_key_t *key) const;

    /// \brief API to walk all the slabs
    /// \param[in] walk_cb    callback to be invoked for every slab
    /// \param[in] ctxt       opaque context passed back to the callback
    /// \return   SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t slab_walk(state_walk_cb_t walk_cb, void *ctxt) override;

    friend void slab_delay_delete_cb(void *timer, uint32_t slab_id, void *elem);

private:
    ht *policy_ht(void) const { return policy_ht_; }
    slab *policy_slab(void) const { return policy_slab_; }
    ht *security_profile_ht(void) const { return security_profile_ht_; }
    slab *security_profile_slab(void) const { return security_profile_slab_; }

    /// policy class is friend of policy_state
    friend class policy;
    /// security profile class is a friend of policy_state
    friend class security_profile;

private:
    ht      *policy_ht_;                ///< security policy database
    slab    *policy_slab_;              ///< slab to allocate security policy instance
    ht      *security_profile_ht_;      ///< security profile database
    slab    *security_profile_slab_;    ///< slab to allocate security profile
    sdk::lib::kvstore *kvstore_;        ///< key-value store instance
};

static inline policy *
policy_find (pds_obj_key_t *key)
{
    return (policy *)api_base::find_obj(OBJ_ID_POLICY, key);
}

static inline security_profile *
security_profile_find (pds_obj_key_t *key)
{
    return (security_profile *)api_base::find_obj(OBJ_ID_SECURITY_PROFILE, key);
}

/// \@}    // end of PDS_POLICY_STATE

}    // namespace api

using api::policy_state;

#endif    // __POLICY_STATE_HPP__
