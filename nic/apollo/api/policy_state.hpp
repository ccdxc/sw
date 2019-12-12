/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    policy_state.hpp
 *
 * @brief   policy database handling
 */

#if !defined (__POLICY_STATE_HPP__)
#define __POLICY_STATE_HPP__

#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/policy.hpp"

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
    /**
     * @brief    constructor
     */
    policy_state();

    /**
     * @brief    destructor
     */
    ~policy_state();

    /**
     * @brief    allocate memory required for a security policy instance
     * @return pointer to the allocated security policy instance,
     *         or NULL if no memory
     */
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

    /**
     * @brief      free security policy instance back to slab
     * @param[in]  policy   pointer to the allocated security
     *             policy instance
     */
    void free(policy *policy);

    /**
     * @brief     lookup a security policy in database given the key
     * @param[in] policy_key security policy key
     */
    policy *find(pds_policy_key_t *policy_key) const;

    /// \brief API to walk all the slabs
    /// \param[in] walk_cb    callback to be invoked for every slab
    /// \param[in] ctxt       opaque context passed back to the callback
    /// \return   SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t slab_walk(state_walk_cb_t walk_cb, void *ctxt) override;

    friend void slab_delay_delete_cb(void *timer, uint32_t slab_id, void *elem);

private:
    ht *policy_ht(void) const { return policy_ht_; }
    slab *policy_slab(void) const { return policy_slab_; }
    friend class policy;   /**< policy class is friend of policy_state */

private:
    ht      *policy_ht_;      /**< security policy database */
    slab    *policy_slab_;    /**< slab to allocate security policy instance */
};

static inline policy *
policy_find (pds_policy_key_t *key) {
    return (policy *)api_base::find_obj(OBJ_ID_POLICY, key);
}

/** @} */    // end of PDS_POLICY_STATE

}    // namespace api

using api::policy_state;

#endif    /** __POLICY_STATE_HPP__ */
