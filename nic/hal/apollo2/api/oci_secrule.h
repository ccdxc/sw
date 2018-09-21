/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci_secrule.h
 *
 * @brief   This module defines OCI Security Rule interface
 */

#if !defined (__OCI_SECRULE_H_)
#define __OCI_SECRULE_H_

#include <oci_types.h>

/**
 * @defgroup OCI_SECRULE- Security Rule specific API definitions
 *
 * @{
 */

/**
 * @brief Layer4 port match
 */
typedef struct _oci_secrule_l4_port_match_t
{
    oci_l4_port_range_t sport_range;    /**< Source port range */
    oci_l4_port_range_t dport_range;    /**< Destination port range */

} oci_secrule_l4_port_match_t;

/**
 * @brief ICMP match
 */
typedef struct _oci_secrule_icmp_match_t
{
    oci_uint8_t icmp_type;    /**< ICMP type */
    oci_uint8_t icmp_code;    /**< ICMP code */

} oci_secrule_icmp_match_t;

/**
 * @brief Security rule layer4 match
 */
typedef struct _oci_secrule_l4_match_t
{
    union {
        oci_secrule_l4_port_match_t port_match;    /**< Layer4 port match */
        oci_secrule_icmp_match_t icmp_match;       /**< ICMP match */
    };

} oci_secrule_l4_match_t;

/**
 * @brief Security rule match
 */
typedef struct _oci_sec_rule_match_t
{
    oci_ip_prefix_t src_ip;             /**< Source IP prefix */
    oci_ip_prefix_t dst_ip;             /**< Destination IP prefix */
    oci_uint8_t ip_proto;               /**< IP protocol */
    oci_secrule_l4_match_t l4_match;    /**< Layer4 match */

} oci_secrule_match_t;

/**
 * @brief Security rule action
 */
typedef struct _oci_secrule_action_t
{
    SECRULE_ACTION_DENY = 0,    /**< Deny */
    SECRULE_ACTION_ALLOW,       /**< Allow */

} oci_secrule_action_t;

/**
 * @brief Security Rule
 */
typedef struct _oci_secrule_t
{
    uint32_t stateful:1;            /**< TRUE if rule is stateful */
    oci_secrule_match_t match;      /**< Rule match */
    oci_secrule_action_t action;    /**< Rule action */

} oci_secrule_t;

/**
 * @brief Security rule list
 */
typedef struct _oci_secrule_list_t
{
    oci_uint32_t count;               /**< Number of rules in the list */
    oci_secrule_t secrule_list[0];    /**< List of rules */

} oci_secrule_list_t;

/**
 * @brief Security rule group key
 */
typedef struct _oci_secrule_grp_key_t
{
    oci_subnet_id_t subnet_id;    /**< Subnet ID */

} PACKED oci_secrule_grp_key_t;

/**
 * @brief Security rule group
 */
typedef struct _oci_secrule_grp_t
{
    oci_secrule_grp_key_t key;          /**< Security rule group key */
    oci_secrule_list_t secrule_list;    /**< List of security rules in
                                             the group */
} PACKED oci_secrule_grp_t;

/**
 * @brief Create security rules group 
 *
 * @param[in] secrule_grp Security Rules Group information
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_secrule_grp_create (
        _In_ oci_secrule_grp_t *secrule_grp);


/**
 * @brief Delete security rules group
 *
 * @param[in] secrule_grp_key Security Rule Group Key
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_secrule_grp_delete (
        _In_ oci_secrule_grp_key_t *secrule_grp_key);

/**
 * @}
 */
#endif /** __OCI_SECRULE_H_ */
