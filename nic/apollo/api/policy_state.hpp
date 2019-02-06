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
#include "nic/apollo/api/policy.hpp"

namespace api {

/**
 * @defgroup OCI_POLICY_STATE - security policy state/db functionality
 * @ingroup OCI_POLICY
 * @{
 */

/**
 * @brief    state maintained for security policies
 */
class policy_state : public obj_base {
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
    policy *policy_alloc(void);

    /**
     * @brief      free security policy instance back to slab
     * @param[in]  policy   pointer to the allocated security
     *             policy instance
     */
    void policy_free(policy *policy);

    /**
     * @brief     lookup a security policy in database given the key
     * @param[in] policy_key security policy key
     */
    policy *policy_find(oci_policy_key_t *policy_key) const;

    friend void slab_delay_delete_cb(void *timer, uint32_t slab_id, void *elem);

private:
    ht *policy_ht(void) { return policy_ht_; }
    slab *rout_table_slab(void) { return policy_slab_; }
    friend class policy;   /**< policy class is friend of policy_state */

private:
    ht      *policy_ht_;      /**< security policy database */
    slab    *policy_slab_;    /**< slab to allocate security policy instance */
};

/** @} */    // end of OCI_POLICY_STATE

}    // namespace api

using api::policy_state;

#endif    /** __POLICY_STATE_HPP__ */
