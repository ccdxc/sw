/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    rfc.hpp
 *
 * @brief   external APIs for Recursive Flow Classficiation (RFC) library
 */

#if !defined (__RFC_HPP__)
#define __RFC_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/apollo/api/include/pds_policy.hpp"

namespace rfc {

/**< policy is list of rules */
typedef struct policy_s {
    uint8_t            af;             /**< address family */
    uint32_t           max_rules;      /**< max size of policy table */
    rule_action_data_t default_action; ///< default action for this policy
    uint32_t           num_rules;      /**< number of rules */
    rule_t             *rules;         /**< rule list */
} policy_t;

/**< RFC policy related params for creation of the policy */
typedef struct policy_params_s {
    /**< policy consisting of rules */
    policy_t policy;
    /**< memory address at which tree should be built */
    mem_addr_t rfc_tree_root_addr;
    /**< RFC memory block size reserved for this policy (for error
     *   detection
     */
    uint32_t rfc_mem_size;
} policy_params_t;

/**
 * @brief    build interval tree based RFC LPM trees and index tables for
 *           subsequent RFC phases, starting at the given memory address
 * @param[in] policy_params    pointer to the policy related parameters
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t rfc_policy_create(policy_params_t *params);

/**
* @brief    dump a given policy rule
* @param[in]    policy    policy table
* @param[in]    rule_num  rule number of the rule to dump
*/
void rfc_policy_rule_dump(policy_t *policy, uint32_t rule_num);

}    // namespace rfc

#endif    /** __RFC_HPP__ */
