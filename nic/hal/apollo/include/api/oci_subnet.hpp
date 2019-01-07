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
#include "nic/hal/apollo/include/api/oci.hpp"
#include "nic/hal/apollo/include/api/oci_vcn.hpp"
#include "nic/hal/apollo/include/api/oci_route.hpp"

/**
 * @defgroup OCI_SUBNET - subnet specific API definitions
 *
 * @{
 */

// TODO: we have to confirm this number !!!
#define OCI_MAX_SUBNET        (5 * OCI_MAX_VCN)

/**
 * @brief subnet Key
 */
typedef struct oci_subnet_key_s {
    oci_vcn_id_t       vcn_id;    /**< vcn id */
    oci_subnet_id_t    id;        /**< subnet id */
} __PACK__ oci_subnet_key_t;

/**
 * @brief subnet
 */
typedef struct oci_subnet_s {
    oci_subnet_key_t         key;            /**< subnet key */
    ip_prefix_t              pfx;            /**< subnet cidr block */
    ip_addr_t                vr_ip;          /**< virtual router IP */
    mac_addr_t               vr_mac;         /**< virtual router mac */
    oci_route_table_key_t    route_table;    /**< route table id */
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
