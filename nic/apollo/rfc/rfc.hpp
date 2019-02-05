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
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/include/sdk/l4.hpp"
#include "nic/apollo/include/api/oci_security_policy.hpp"

namespace rfc {

/**< type of the policy */
enum {
    POLICY_TYPE_NONE     = 0,
    POLICY_TYPE_FIREWALL = 1,
};

/**< rule action */ 
typedef struct rule_action_s {
    union {
        security_rule_action_t    fw_action;
    };
} rule_action_t;

/**< generic rule definition */
typedef struct rule_s {
    rule_match_t     match;
    rule_action_t    action;
} rule_t;

/**< policy is list of rules */
typedef struct policy_s {
    uint8_t     af;           /**< address family */
    uint32_t    max_rules;    /**< max size of policy table */
    uint32_t    num_rules;    /**< number of rules */
    rule_t      rules[0];     /**< rule list */
} policy_t;

/**
 * @brief    build interval tree based RFC LPM trees and index tables for
 *           subsequent RFC phases, starting at the given memory address
 * @param[in] policy           pointer to the policy
 * @param[in] rfc_tree_root    pointer to the memory address at which tree
 *                             should be built
 * @param[in] rfc_mem_size     RFC memory block size provided (for error
 *                             detection)
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t rfc_create(policy_t *policy, mem_addr_t rfc_tree_root_addr,
                     uint32_t rfc_mem_size);

}    // namespace rfc

#endif    /** __RFC_HPP__ */
