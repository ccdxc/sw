/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci_subnet.hpp
 *
 * @brief   This module defines subnet interface
 */

#if !defined (__OCI_SUBNET_HPP__)
#define __OCI_SUBNET_HPP__

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/include/api/oci.hpp"
#include "nic/apollo/include/api/oci_vcn.hpp"
#include "nic/apollo/include/api/oci_route.hpp"
#include "nic/apollo/include/api/oci_policy.hpp"

/**
 * @defgroup OCI_SUBNET - subnet specific API definitions
 *
 * @{
 */

// TODO: we have to confirm this number !!!
#define OCI_MAX_SUBNET        (5 * OCI_MAX_VCN)

/**
 * @brief subnet key
 *
 * NOTE: subnet key is globally unique id, not scoped with in a vcn
 */
typedef struct oci_subnet_key_s {
    oci_subnet_id_t    id;        /**< subnet id */
} __PACK__ oci_subnet_key_t;

/**
 * @brief subnet
 */
typedef struct oci_subnet_s {
    oci_subnet_key_t         key;            /**< subnet key */
    oci_vcn_key_t            vcn;            /**< vcn this subnet is part of */
    ip_prefix_t              pfx;            /**< subnet cidr block */
    ip_addr_t                vr_ip;          /**< virtual router IP */
    mac_addr_t               vr_mac;         /**< virtual router mac */
    oci_route_table_key_t    v4_route_table; /**< IPv4 route table key */
    oci_route_table_key_t    v6_route_table; /**< IPv6 route table key */
    oci_policy_key_t         ing_v4_policy;  /**< ingress IPv4 policy table */
    oci_policy_key_t         ing_v6_policy;  /**< ingress IPv6 policy table */
    oci_policy_key_t         egr_v4_policy;  /**< egress IPv4 policy table */
    oci_policy_key_t         egr_v6_policy;  /**< egress IPv6 policy table */
} __PACK__ oci_subnet_t;

/**
 * @brief Create subnet
 *
 * @param[in] subnet subnet information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t oci_subnet_create(_In_ oci_subnet_t *subnet);

/**
 * @brief Delete subnet
 *
 * @param[in] subnet_key subnet key
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t oci_subnet_delete(_In_ oci_subnet_key_t *subnet_key);

/**
 * @}
 */
#endif /** __OCI_SUBNET_HPP__ */
