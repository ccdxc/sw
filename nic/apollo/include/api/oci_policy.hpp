/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci_policy.hpp
 *
 * @brief   This module defines policy interface like security rules
 */

#if !defined (__OCI_POLICY_HPP__)
#define __OCI_POLICY_HPP__

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/include/sdk/l4.hpp"
#include "nic/sdk/include/sdk/mem.hpp"
#include "nic/apollo/include/api/oci.hpp"

/**
 * @defgroup OCI_POLICY - policy specific API definitions
 *
 * @{
 */

#define OCI_MAX_SECURITY_POLICY                  1024
#define OCI_MAX_RULES_PER_SECURITY_POLICY        1024

#define OCI_MAX_POLICY        OCI_MAX_SECURITY_POLICY

/**
 * @brief rule direction
 */
typedef enum rule_dir_e {
    RULE_DIR_INGRESS =  0,    /**< ingress direction */
    RULE_DIR_EGRESS  =  1,    /**< egress direction */
} rule_dir_t;

/**
 * @brief policy key
 */
typedef struct oci_policy_key_s {
    oci_policy_id_t    id;    /**< unique id for the policy */
} __PACK__ oci_policy_key_t;

/**
 * @brief rule L3 match criteria
 */
typedef struct rule_l3_match_s {
    uint8_t        ip_proto;    /**< IP protocol */
    ip_prefix_t    ip_pfx;      /**< IP prefix */
} __PACK__ rule_l3_match_t;

/**
 * @brief rule L4 match criteria
 */
typedef struct rule_l4_match_s {
    union {
        struct {
            port_range_t    sport_range;    /**< source port range */
            port_range_t    dport_range;    /**< destination port range */
        };
        struct {
            uint8_t    icmp_type;    /**< ICMP type */
            uint8_t    icmp_code;    /**< ICMP code */
        };
    };
} __PACK__ rule_l4_match_t;

/**
 * @brief rule match
 */
typedef struct rule_match_s {
    rule_l3_match_t    l3_match;    /**< Layer 3 match criteria */
    rule_l4_match_t    l4_match;    /**< Layer 4 match criteria */
} __PACK__ rule_match_t;

/**
 * @brief security rule action
 */
typedef enum fw_action_e {
    SECURITY_RULE_ACTION_DENY  = 0,    /**< deny/drop the packet */
    SECURITY_RULE_ACTION_ALLOW = 1,    /**< allow the packet */
} fw_action_t;

/**
 * @brief security rule specific action data
 */
typedef struct fw_action_data_s {
    fw_action_t    action;
} fw_action_data_t;

/**
 * @brief generic rule action data
 */
typedef union rule_action_data_s {
    fw_action_data_t    fw_action;    /**< firewall action data */
} rule_action_data_t;

/**
 * @brief generic rule
 */
typedef struct rule_s {
    bool                  stateful;       /**< true, if rule is stateful */
    rule_match_t          match;          /**< rule match */
    rule_action_data_t    action_data;    /**< action and related information */
} __PACK__ rule_t;

/**< type of the policy */
typedef enum policy_type_s {
    POLICY_TYPE_NONE     = 0,
    POLICY_TYPE_FIREWALL = 1,
} policy_type_t;

/**
 * @brief generic policy structure
 */
typedef struct oci_policy_s    oci_policy_t;
struct oci_policy_s {
    oci_policy_key_t    key;            /**< policy key */
    policy_type_t       policy_type;    /**< type of policy */
    uint8_t             af;             /**< IP_AF_IPV4 or IP_AF_IPV6 */
    rule_dir_t          direction;      /**< policy enforcement direction */
    uint32_t            num_rules;      /**< number of rules in the list */
    rule_t              *rules;     /**< list or rules */

    oci_policy_s() { rules = NULL; }
    ~oci_policy_s() {
        if (rules) {
            SDK_FREE(OCI_MEM_ALLOC_SEUCURITY_POLICY, rules);
        }
    }
    oci_policy_t& operator= (const oci_policy_t& policy) {
        /**< self-assignment guard */
        if (this == &policy) {
            return *this;
        }
        key = policy.key;
        direction = policy.direction;
        num_rules = policy.num_rules;
        if (rules) {
            SDK_FREE(OCI_MEM_ALLOC_SEUCURITY_POLICY, rules);
        }
        rules =
            (rule_t *)SDK_MALLOC(OCI_MEM_ALLOC_SEUCURITY_POLICY,
                                          num_rules * sizeof(rule_t));
        memcpy(rules, policy.rules, num_rules * sizeof(rule_t));
        return *this;
    }
} __PACK__;

/**
 * @brief create policy
 *
 * @param[in] policy    policy information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t oci_policy_create(_In_ oci_policy_t *policy);


/**
 * @brief delete policy
 *
 * @param[in] policy_key    policy key
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t oci_policy_delete(_In_ oci_policy_key_t *policy_key);

/**
 * @}
 */

#endif    /** __OCI_POLICY_HPP__ */
