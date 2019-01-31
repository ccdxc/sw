/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci_security_policy.hpp
 *
 * @brief   This module defines security policy interface
 */

#if !defined (__OCI_SECURITY_POLICY_HPP__)
#define __OCI_SECURITY_POLICY_HPP__

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/include/sdk/l4.hpp"
#include "nic/sdk/include/sdk/mem.hpp"
#include "nic/apollo/include/api/oci.hpp"

/**
 * @defgroup OCI_SECURITY_POLICY - security policy specific API definitions
 *
 * @{
 */

#define OCI_MAX_SECURITY_POLICY                  1024
#define OCI_MAX_RULES_PER_SECURITY_POLICY        1024

/**
 * @brief rule direction
 */
typedef enum rule_dir_e {
    RULE_DIR_INGRESS =  0,    /**< ingress direction */
    RULE_DIR_EGRESS  =  1,    /**< egress direction */
} rule_dir_t;

/**
 * @brief security policy key
 */
typedef struct oci_security_policy_key_s {
    oci_security_policy_id_t    id;    /**< unique id for the security policy */
} __PACK__ oci_security_policy_key_t;

/**
 * @brief security rule L4 match
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
    uint8_t             ip_proto;    /**< IP protocol */
    ip_prefix_t         ip;          /**< IP prefix */
    rule_l4_match_t     l4_match;    /**< Layer4 match */
} __PACK__ rule_match_t;

/**
 * @brief security rule action
 */
typedef enum security_rule_action_e {
    SECURITY_RULE_ACTION_DENY  = 0,    /**< deny/drop the packet */
    SECURITY_RULE_ACTION_ALLOW = 1,    /**< allow the packet */
} security_rule_action_t;

/**
 * @brief security rule
 */
typedef struct security_rule_s {
    bool                      stateful;    /**< true, if rule is stateful */
    rule_match_t              match;       /**< rule match */
    security_rule_action_t    action;      /**< rule action */
} __PACK__ security_rule_t;

/**
 * @brief security rule list
 */
typedef struct oci_security_policy_s    oci_security_policy_t;
struct oci_security_policy_s {
    oci_security_policy_key_t    key;        /**< security policy key */
    rule_dir_t                   direction;  /**< security policy enforcement direction */
    uint32_t                     num_rules;  /**< number of rules in the list */
    security_rule_t              *rules;     /**< list or rules */

    oci_security_policy_s() { rules = NULL; }
    ~oci_security_policy_s() {
        if (rules) {
            SDK_FREE(OCI_MEM_ALLOC_SEUCURITY_POLICY, rules);
        }
    }
    oci_security_policy_t& operator= (const oci_security_policy_t& policy) {
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
            (security_rule_t *)SDK_MALLOC(OCI_MEM_ALLOC_SEUCURITY_POLICY,
                                          num_rules * sizeof(security_rule_t));
        memcpy(rules, policy.rules, num_rules * sizeof(security_rule_t));
        return *this;
    }
} __PACK__;

/**
 * @brief create security policy
 *
 * @param[in] policy    security policy information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t oci_security_policy_create(_In_ oci_security_policy_t *policy);


/**
 * @brief delete security policy
 *
 * @param[in] policy_key    security policy key
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t oci_security_policy_delete(_In_ oci_security_policy_key_t *policy_key);

/**
 * @}
 */

#endif    /** __OCI_SECURITY_POLICY_HPP__ */
