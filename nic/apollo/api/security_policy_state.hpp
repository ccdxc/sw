/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    security_policy_state.hpp
 *
 * @brief   security policy database handling
 */

#if !defined (__SECURITY_POLICY_STATE_HPP__)
#define __SECURITY_POLICY_STATE_HPP__

#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/api/security_policy.hpp"

namespace api {

/**
 * @defgroup OCI_SECURITY_POLICY_STATE - security policy state/db functionality
 * @ingroup OCI_SECURITY_POLICY
 * @{
 */

/**
 * @brief    state maintained for security policies
 */
class security_policy_state : public obj_base {
public:
    /**
     * @brief    constructor
     */
    security_policy_state();

    /**
     * @brief    destructor
     */
    ~security_policy_state();

    /**
     * @brief    allocate memory required for a security policy instance
     * @return pointer to the allocated security policy instance,
     *         or NULL if no memory
     */
    security_policy *security_policy_alloc(void);

    /**
     * @brief      free security policy instance back to slab
     * @param[in]  security_policy   pointer to the allocated security
     *             policy instance
     */
    void security_policy_free(security_policy *policy);

    /**
     * @brief     lookup a security policy in database given the key
     * @param[in] policy_key security policy key
     */
    security_policy *security_policy_find(oci_security_policy_key_t *policy_key) const;

    friend void slab_delay_delete_cb(void *timer, uint32_t slab_id, void *elem);

private:
    ht *security_policy_ht(void) { return security_policy_ht_; }
    slab *rout_table_slab(void) { return security_policy_slab_; }
    friend class security_policy;   /**< security_policy class is friend of security_policy_state */

private:
    ht      *security_policy_ht_;      /**< security policy database */
    slab    *security_policy_slab_;    /**< slab to allocate security policy instance */
};

/** @} */    // end of OCI_SECURITY_POLICY_STATE

}    // namespace api

using api::security_policy_state;

#endif    /** __SECURITY_POLICY_STATE_HPP__ */
