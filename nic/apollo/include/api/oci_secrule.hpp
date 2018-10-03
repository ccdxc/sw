/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci_secrule.hpp
 *
 * @brief   This module defines OCI Security Rule interface
 */

#if !defined (__OCI_SECRULE_H_)
#define __OCI_SECRULE_H_

#include <oci_types.h>

/**
 * @defgroup OCI_SECRULE - Security Rule specific API definitions
 *
 * @{
 */

/**
 * @brief Security Rule Direction
 */
typedef enum oci_secrule_dir_e {
    OCI_SECRULE_DIR_INGRESS    =  0,    /**< Ingress Direction */
    OCI_SECRULE_DIR_EGRESS     =  1,    /**< Egress Direction */
} oci_secrule_dir_t;

/**
 * @brief Layer4 port match
 */
typedef struct oci_secrule_l4_port_match_s {
    oci_l4_port_range_t sport_range;    /**< Source port range */
    oci_l4_port_range_t dport_range;    /**< Destination port range */
} __PACK__ oci_secrule_l4_port_match_t;

/**
 * @brief ICMP match
 */
typedef struct oci_secrule_icmp_match_s {
    oci_uint8_t icmp_type;    /**< ICMP type */
    oci_uint8_t icmp_code;    /**< ICMP code */
} __PACK__ oci_secrule_icmp_match_t;

/**
 * @brief Security rule layer4 match
 */
typedef struct oci_secrule_l4_match_s {
    union {
        oci_secrule_l4_port_match_t port_match;    /**< Layer4 port match */
        oci_secrule_icmp_match_t icmp_match;       /**< ICMP match */
    };
} __PACK__ oci_secrule_l4_match_t;

/**
 * @brief Security rule match
 */
typedef struct oci_sec_rule_match_s {
    oci_ip_prefix_t ip;                 /**< IP prefix */
    oci_uint8_t ip_proto;               /**< IP protocol */
    oci_secrule_l4_match_t l4_match;    /**< Layer4 match */
} __PACK__ oci_secrule_match_t;

/**
 * @brief Security rule action
 */
typedef enum oci_secrule_action_e {
    SECRULE_ACTION_DENY    =  0,    /**< Deny */
    SECRULE_ACTION_ALLOW   =  1,    /**< Allow */
} oci_secrule_action_t;

/**
 * @brief Security rule
 */
typedef struct oci_secrule_s {
    uint32_t stateful:1;            /**< TRUE if rule is stateful */
    oci_secrule_match_t match;      /**< Rule match */
    oci_secrule_action_t action;    /**< Rule action */
} __PACK__ oci_secrule_t;

/**
 * @brief Security rule list
 */
typedef struct oci_secrule_list_s {
    oci_uint32_t count;               /**< Number of rules in the list */
    oci_secrule_t secrule_list[0];    /**< List or rules */
} __PACK__ oci_secrule_list_t;

/**
 * @brief Security rule Group key
 */
typedef struct oci_secrule_grp_key_s {
    oci_subnet_id_t subnet_id;    /**< Subnet ID */
} __PACK__ oci_secrule_grp_key_t;

/**
 * @brief Security rule Group
 */
typedef struct oci_secrule_grp_s {
    oci_secrule_grp_key_t key;          /**< Route group key */
    oci_secrule_dir_t dir;              /**< Ingress/Egress */
    oci_secrule_list_t secrule_list;    /**< List of routes in the group */
} __PACK__ oci_secrule_grp_t;

/**
 * @brief Create security rules group
 *
 * @param[in] secrule_grp Security rules Group information
 *
 * @return #SDK_RET_OK on success, failure status code on error
 */
oci_status_t oci_secrule_grp_create (
    _In_ oci_secrule_grp_t *secrule_grp);


/**
 * @brief Delete security rules group
 *
 * @param[in] secrule_grp_key Security rules Group Key
 *
 * @return #SDK_RET_OK on success, failure status code on error
 */
oci_status_t oci_secrule_grp_delete (
    _In_ oci_secrule_grp_t *secrule_grp);

/**
 * @}
 */
#endif /** __OCI_SECRULE_H_ */
