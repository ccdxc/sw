//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines policy API
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_PDS_POLICY_HPP__
#define __INCLUDE_API_PDS_POLICY_HPP__

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/include/sdk/l4.hpp"
#include "nic/sdk/include/sdk/mem.hpp"
#include "nic/apollo/include/api/pds.hpp"

/// \defgroup PDS_POLICY Policy API
/// @{

#define PDS_MAX_SECURITY_POLICY                  1024
#define PDS_MAX_RULES_PER_SECURITY_POLICY        1023

#define PDS_MAX_POLICY        PDS_MAX_SECURITY_POLICY

/// \brief Rule direction
typedef enum rule_dir_e {
    RULE_DIR_INGRESS =  0,    ///< Ingress direction
    RULE_DIR_EGRESS  =  1,    ///< Egress direction
} rule_dir_t;

/// \brief Policy key
typedef struct pds_policy_key_s {
    pds_policy_id_t    id;    ///< Unique ID for the policy
} __PACK__ pds_policy_key_t;

/// \brief Rule L3 match criteria
typedef struct rule_l3_match_s {
    uint8_t        ip_proto;    ///< IP protocol
    ip_prefix_t    ip_pfx;      ///< IP prefix
} __PACK__ rule_l3_match_t;

/// \brief Rule L4 match criteria
typedef struct rule_l4_match_s {
    union {
        struct {
            port_range_t    sport_range;    ///< source port range
            port_range_t    dport_range;    ///< destination port range
        };
        struct {
            uint8_t    icmp_type;    ///< ICMP type
            uint8_t    icmp_code;    ///< ICMP code
        };
    };
} __PACK__ rule_l4_match_t;

/// \brief Rule match
typedef struct rule_match_s {
    rule_l3_match_t    l3_match;    ///< Layer 3 match criteria
    rule_l4_match_t    l4_match;    ///< Layer 4 match criteria
} __PACK__ rule_match_t;

/// \brief Security rule action
typedef enum fw_action_e {
    SECURITY_RULE_ACTION_ALLOW = 0,    ///< Allow the packet
} fw_action_t;

/// \brief Security rule specific action data
typedef struct fw_action_data_s {
    fw_action_t    action;    ///< Firewall action
} fw_action_data_t;

/// \brief Generic rule action data
typedef union rule_action_data_s {
    fw_action_data_t    fw_action;    ///< Firewall action data
} rule_action_data_t;

/// \brief Generic rule
typedef struct rule_s {
    bool                  stateful;       ///< true, if rule is stateful
    rule_match_t          match;          ///< rule match
    rule_action_data_t    action_data;    ///< action and related information
} __PACK__ rule_t;

/// \brief Policy type
typedef enum policy_type_s {
    POLICY_TYPE_NONE     = 0,
    POLICY_TYPE_FIREWALL = 1,
} policy_type_t;

/// \brief Generic policy
typedef struct pds_policy_spec_s    pds_policy_spec_t;
struct pds_policy_spec_s {
    pds_policy_key_t    key;            ///< policy key
    policy_type_t       policy_type;    ///< type of policy
    uint8_t             af;             ///< Address family
    rule_dir_t          direction;      ///< Policy enforcement direction
    uint32_t            num_rules;      ///< Number of rules in the list
    rule_t              *rules;         ///< List or rules

    pds_policy_spec_s() { rules = NULL; }
    ~pds_policy_spec_s() {
        if (rules) {
            SDK_FREE(PDS_MEM_ALLOC_SEUCURITY_POLICY, rules);
        }
    }
    pds_policy_spec_t& operator= (const pds_policy_spec_t& policy) {
        // self-assignment guard
        if (this == &policy) {
            return *this;
        }
        key = policy.key;
        policy_type = policy.policy_type;
        af = policy.af;
        direction = policy.direction;
        num_rules = policy.num_rules;
        if (rules) {
            SDK_FREE(PDS_MEM_ALLOC_SEUCURITY_POLICY, rules);
        }
        rules =
            (rule_t *)SDK_MALLOC(PDS_MEM_ALLOC_SEUCURITY_POLICY,
                                          num_rules * sizeof(rule_t));
        memcpy(rules, policy.rules, num_rules * sizeof(rule_t));
        return *this;
    }
} __PACK__;

/// \brief Create policy
///
/// \param[in] policy Policy information
//
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_policy_create(pds_policy_spec_t *policy);

/// \brief Delete policy
///
/// \param[in] key Key
///
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_policy_delete(_In_ pds_policy_key_t *key);

/// \@}

#endif    // __INCLUDE_API_PDS_POLICY_HPP__
