/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    security_policy_impl_state.hpp
 *
 * @brief   security policy implementation state
 */
#if !defined (__SECURITY_POLICY_STATE_IMPL_HPP__)
#define __SECURITY_POLICY_IMPL_STATE_HPP__

#include "nic/sdk/lib/table/directmap/directmap.hpp"
#include "nic/sdk/lib/table/hash/hash.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/api/oci_state.hpp"

namespace impl {

/**
 * @defgroup OCI_SECURITY_POLICY_IMPL_STATE - security policy impl state
 *                                            functionality
 * @ingroup OCI_SECURITY_POLICY
 * @{
 */

 /**< forward declaration */
class security_policy_impl;

/**
 * @brief    state maintained for security policies
 */
class security_policy_impl_state : public obj_base {
public:
    /**< @brief    constructor */
    security_policy_impl_state(oci_state *state);

    /**< @brief    destructor */
    ~security_policy_impl_state();

    /**< @brief    return policy region's base/start address in memory */
    mem_addr_t security_policy_region_addr(void) const {
        return security_policy_region_addr_;
    }

    /**< @brief    return per security policy table's size */
    mem_addr_t security_policy_table_size(void) const {
        return security_policy_table_size_;
    }

private:
    indexer *security_policy_idxr(void) { return security_policy_idxr_; }
    friend class security_policy_impl;

private:
    /**< datapath tables for security policy */
    indexer       *security_policy_idxr_;        /**< indexer to allocate mem block */
    mem_addr_t    security_policy_region_addr_;  /**< base address for the policy region */
    uint32_t      security_policy_table_size_;   /**< size of each policy table */
};

/** * @} */    // end of OCI_SECURITY_POLICY_IMPL_STATE

}    // namespace impl

#endif    /** __SECURITY_POLICY_IMPL_STATE_HPP__ */
