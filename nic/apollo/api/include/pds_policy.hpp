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
#include "nic/apollo/api/include/pds.hpp"

/// \defgroup PDS_POLICY Policy API
/// @{

#define PDS_MAX_SECURITY_POLICY                  1023    ///< Maximum security policies
#define PDS_MAX_RULES_PER_IPV4_SECURITY_POLICY   1023    ///< Maximum IPV4 rules per policy
#define PDS_MAX_RULES_PER_IPV6_SECURITY_POLICY     15    ///< Maximum IPV6 rules per policy

/// \brief    rule direction
typedef enum rule_dir_e {
    RULE_DIR_INGRESS = 0,    ///< Ingress direction
    RULE_DIR_EGRESS  = 1,    ///< Egress direction
} rule_dir_t;

/// \brief type of the IP field used in rule match
typedef enum ip_match_type_s {
    IP_MATCH_PREFIX = 0,    ///< match based on the prefix
    IP_MATCH_RANGE  = 1,    ///< match based on the range
    IP_MATCH_TAG    = 2,    ///< match based on the tag
} ip_match_type_t;

/// \brief    rule L3 match criteria
typedef struct rule_l3_match_s {
    uint8_t          ip_proto;        ///< IP protocol
    ip_match_type_t  src_match_type;  ///< src match condition type
    ip_match_type_t  dst_match_type;  ///< dst match condition type
    union {
        ip_prefix_t  src_ip_pfx;      ///< src IP prefix
        ipvx_range_t src_ip_range;    ///< src IP range
        uint32_t     src_tag;         ///< src tag value
    };
    union {
        ip_prefix_t  dst_ip_pfx;      ///< dst IP prefix
        ipvx_range_t dst_ip_range;    ///< drc IP range
        uint32_t     dst_tag;         ///< drc tag value
    };
} __PACK__ rule_l3_match_t;

/// \brief    rule L4 match criteria
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

/// \brief    rule match
typedef struct rule_match_s {
    rule_l3_match_t    l3_match;    ///< Layer 3 match criteria
    rule_l4_match_t    l4_match;    ///< Layer 4 match criteria
} __PACK__ rule_match_t;

/// \brief    security rule action
typedef enum fw_action_e {
    SECURITY_RULE_ACTION_ALLOW = 0,    ///< Allow the packet
    SECURITY_RULE_ACTION_DENY  = 1,    ///< Drop the packet silently
} fw_action_t;

/// \brief    security rule specific action data
typedef struct fw_action_data_s {
    fw_action_t    action;    ///< Firewall action
} fw_action_data_t;

/// \brief    generic rule action data
typedef union rule_action_data_s {
    fw_action_data_t    fw_action;    ///< Firewall action data
} rule_action_data_t;

/// \brief    generic rule
typedef struct rule_s {
    bool                  stateful;       ///< true, if rule is stateful
    uint32_t              priority;       ///< rule Priority
    rule_match_t          match;          ///< rule match
    rule_action_data_t    action_data;    ///< action and related information
} __PACK__ rule_t;

/// \brief    policy type
typedef enum policy_type_s {
    POLICY_TYPE_NONE     = 0,
    POLICY_TYPE_FIREWALL = 1,
} policy_type_t;

/// \brief    generic policy specification
typedef struct pds_policy_spec_s    pds_policy_spec_t;
/// \brief    generic policy specification
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
            SDK_FREE(PDS_MEM_ALLOC_SECURITY_POLICY, rules);
        }
    }
    /// assignment operator
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
            SDK_FREE(PDS_MEM_ALLOC_SECURITY_POLICY, rules);
        }
        rules =
            (rule_t *)SDK_MALLOC(PDS_MEM_ALLOC_SECURITY_POLICY,
                                 num_rules * sizeof(rule_t));
        memcpy(rules, policy.rules, num_rules * sizeof(rule_t));
        return *this;
    }
} __PACK__;

/// \brief policy status
typedef struct pds_policy_status_s {
    // TODO : Only base address of the tree stored in HBM is read
} pds_policy_status_t;

/// \brief policy statistics
typedef struct pds_policy_stats_s {

} pds_policy_stats_t;

/// \brief policy info
typedef struct pds_policy_info_s {
    pds_policy_spec_t spec;         ///< Specification
    pds_policy_status_t status;     ///< Status
    pds_policy_stats_t stats;       ///< Statistics
} pds_policy_info_t;

/// \brief    create policy
/// \param[in] policy    policy configurationn
/// \return    #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_policy_create(pds_policy_spec_t *policy);

/// \brief    read policy
/// \param[in]  key    policy key
/// \param[out] info    policy information
/// \return    #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_policy_read(pds_policy_key_t *key, pds_policy_info_t *info);

/// \brief    update policy
/// \param[in] policy    policy configuration
/// \return    #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_policy_update(pds_policy_spec_t *policy);

/// \brief    delete policy
/// \param[in] key    policy key
/// \return    #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_policy_delete(pds_policy_key_t *key);

/// @}

#endif    // __INCLUDE_API_PDS_POLICY_HPP__
