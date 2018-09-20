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
    /**
     * @brief Source port range
     */
    oci_l4_port_range_t sport_range;

    /**
     * @brief Destination port range
     */
    oci_l4_port_range_t dport_range;

} oci_secrule_l4_port_match_t;

/**
 * @brief ICMP match
 */
typedef struct _oci_secrule_icmp_match_t
{
    /**
     * @brief ICMP Type
     */
    oci_uint8_t icmp_type;

    /**
     * @brief ICMP Code
     */
    oci_uint8_t icmp_code;

} oci_secrule_icmp_match_t;

/**
 * @brief Security rule layer4 match
 */
typedef struct _oci_secrule_l4_match_t
{
    union {
        /**
         * @brief Layer4 port match
         */
        oci_secrule_l4_port_match_t port_match;

        /**
         * @brief ICMP match
         */
        oci_secrule_icmp_match_t icmp_match;
    };

} oci_secrule_l4_match_t;

/**
 * @brief Security rule match
 */
typedef struct _oci_sec_rule_match_t
{
    /**
     * @brief Source IP prefix
     */
    oci_ip_prefix_t src_ip;

    /**
     * @brief Destination IP prefix
     */
    oci_ip_prefix_t dst_ip;

    /**
     * @brief IP protocol
     */
    oci_uint8_t ip_proto;

    /**
     * @brief Layer4 match
     */
    oci_secrule_l4_match_t l4_match;

} oci_secrule_match_t;

/**
 * @brief Security rule action
 */
typedef struct _oci_secrule_action_t
{
    /**
     * @brief Deny 
     */
    SECRULE_ACTION_DENY = 0,

    /**
     * @brief Allow
     */
    SECRULE_ACTION_ALLOW,

} oci_secrule_action_t;

/**
 * @brief Security Rule
 */
typedef struct _oci_secrule_t
{
    /**
     * @brief TRUE if rule is stateful
     */
    uint32_t stateful:1;

    /**
     * @brief Rule Match
     */
    oci_secrule_match_t match;

    /**
     * @brief Rule Action
     */
    oci_secrule_action_t action;

} oci_secrule_t;

/**
 * @brief Security rule list
 */
typedef struct _oci_secrule_list_t
{
    /**
     * @brief Number of rules in the list
     */
    oci_uint32_t count;

    /**
     * @brief List of rules 
     */
    oci_secrule_t secrule_list[0];

} oci_secrule_list_t;

/**
 * @brief Security rule group key
 */
typedef struct _oci_secrule_grp_key_t
{
    /**
     * @brief Subnet ID
     */
    oci_subnet_id_t subnet_id;

} PACKED oci_secrule_grp_key_t;

/**
 * @brief Security rule group
 */
typedef struct _oci_secrule_grp_t
{
    /**
     * @brief Security rule group key
     */
    oci_secrule_grp_key_t key;

    /**
     * @brief List of security rules in the group
     */
    oci_secrule_list_t secrule_list;

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
