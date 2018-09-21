/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci_subnet.h
 *
 * @brief   This module defines OCI Subnet interface
 */

#if !defined (__OCI_SUBNET_H_)
#define __OCI_SUBNET_H_

#include <oci_types.h>

/**
 * @defgroup OCI_SUBNET - Subnet specific API definitions
 *
 * @{
 */

/**
 * @brief Subnet Key
 */
typedef struct _oci_subnet_key_t
{
    oci_vcn_id_t vcn_id;    /**< VCN ID */
    oci_subnet_id_t id;     /**< Subnet ID */

} PACKED oci_subnet_key_t;

/**
 * @brief Subnet
 */
typedef struct _oci_subnet_t
{
    oci_subnet_key_t key;    /**< Subnet key */
    oci_ip_prefix_t pfx;     /**< Subnet CIDR block

} PACKED oci_subnet_t;

/**
 * @brief Create Subnet
 *
 * @param[in] subnet Subnet information
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_subnet_create (
        _In_ oci_subnet_t *subnet);


/**
 * @brief Delete Subnet 
 *
 * @param[in] subnet_key Subnet key
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_subnet_delete (
        _In_ oci_subnet_key_t *subnet_key);

/**
 * @}
 */
#endif /** __OCI_SUBNET_H_ */
