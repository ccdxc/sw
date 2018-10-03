/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci_subnet.hpp
 *
 * @brief   This module defines OCI Subnet interface
 */

#if !defined (__OCI_SUBNET_H_)
#define __OCI_SUBNET_H_

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/include/api/oci_types.hpp"

/**
 * @defgroup OCI_SUBNET - Subnet specific API definitions
 *
 * @{
 */

/**
 * @brief Subnet Key
 */
typedef struct oci_subnet_key_s {
    oci_vcn_id_t vcn_id;    /**< VCN ID */
    oci_subnet_id_t id;     /**< Subnet ID */
} __PACK__ oci_subnet_key_t;

/**
 * @brief Subnet
 */
typedef struct oci_subnet_s {
    oci_subnet_key_t key;    /**< Subnet key */
    ip_prefix_t pfx;         /**< Subnet CIDR block */
    ip_addr_t vr_ip;         /**< Virtual Router IP */
    mac_addr_t vr_mac;       /**< Virtual Router Mac */
} __PACK__ oci_subnet_t;

/**
 * @brief Create Subnet
 *
 * @param[in] subnet Subnet information
 *
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t oci_subnet_create (
    _In_ oci_subnet_t *subnet);


/**
 * @brief Delete Subnet
 *
 * @param[in] subnet_key Subnet key
 *
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t oci_subnet_delete (
    _In_ oci_subnet_key_t *subnet_key);

/**
 * @}
 */
#endif /** __OCI_SUBNET_H_ */
