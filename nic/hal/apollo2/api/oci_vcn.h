/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci_vcn.h
 *
 * @brief   This module defines OCI VCN interface
 */

#if !defined (__OCI_VCN_H_)
#define __OCI_VCN_H_

#include <oci_types.h>

/**
 * @defgroup OCI_VCN - Virtual Cloud Network specific API definitions
 *
 * @{
 */

/**
 * @brief VCN Key
 */
typedef struct _oci_vcn_key_t
{
    /**
     * @brief VCN ID
     */
    oci_vcn_id_t id;

} PACKED oci_vcn_key_t;

/**
 * @brief VCN
 */
typedef struct _oci_vcn_t
{
    /**
     * @brief VCN Key
     */
    oci_vcn_key_t key;

    /**
     * @brief VCN CIDR block
     */
    oci_ip_prefix_t pfx;

} PACKED oci_vcn_t;

/**
 * @brief Create VCN
 *
 * @param[in] vcn VCN information
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_vcn_create (
        _In_ oci_vcn_t *vcn);


/**
 * @brief Delete VCN
 *
 * @param[in] vcn_key VCN key
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_vcn_delete (
        _In_ oci_vcn_key_t *vcn_key);

/**
 * @}
 */
#endif /** __OCI_VCN_H_ */
