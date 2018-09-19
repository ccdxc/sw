/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    vcn.h
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
 * @brief Attribute data for #OCI_VCN_ATTR_OPER_STATUS
 */
typedef enum _oci_vcn_oper_status_t
{
    /** Unknown */
    OCI_VCN_OPER_STATUS_UNKNOWN,

    /** Up */
    OCI_VCN_OPER_STATUS_UP,

    /** Down */
    OCI_VCN_OPER_STATUS_DOWN,

} oci_vcn_oper_status_t;

/**
 * @brief Attribute Id in oci_vcn_create(), oci_vcn_attribute_set() and
 * oci_vcn_attribute_get() calls
 */
typedef enum _oci_vcn_attr_t
{
    /**
     * @brief Start of attributes
     */
    OCI_VCN_ATTR_START,

    /* READ-ONLY */

    /**
     * @brief Operational Status
     *
     * @type oci_vcn_oper_status_t
     * @flags READ_ONLY
     */
    OCI_VCN_ATTR_OPER_STATUS = OCI_VCN_ATTR_START,

    /* READ-WRITE */

    /**
     * @brief VCN ID
     *
     * @type oci_vnc_id_t
     * @flags MANDATORY_ON_CREATE | CREATE_ONLY | KEY
     */
    OCI_VCN_ATTR_ID,

    /**
     * @brief CIDR block
     *
     * @type oci_ip_prefix_t
     * @flags CREATE_AND_SET
     * @default wildcard
     */
    OCI_VCN_ATTR_CIDR_BLOCK,

    /**
     * @brief End of attributes
     */
    OCI_VCN_ATTR_END,

} oci_vcn_attr_t;

/**
 * @brief Create VCN
 *
 * @param[out] vcn_id VCN id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_vcn_create (
        _Out_ oci_object_id_t *vcn_id,
        _In_ uint32_t attr_count,
        _In_ const oci_attribute_t *attr_list);

/**
 * @brief Set vcn attribute value.
 *
 * @param[in] vcn_id VCN id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_vcn_attribute_set (
        _In_ oci_object_id_t vcn_id,
        _In_ uint32_t attr_count,
        _In_ const oci_attribute_t *attr_list);

/**
 * @brief Get VCN attribute value.
 *
 * @param[in] vcn_id VCN id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_vcn_attribute_get (
        _In_ oci_object_id_t vcn_id,
        _In_ uint32_t attr_count,
        _Inout_ oci_attribute_t *attr_list);

/**
 * @brief Delete VCN
 *
 * @param[in] vcn_id VCN id
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_delete_vcn_delete (
        _In_ oci_object_id_t vcn_id);

/**
 * @}
 */
#endif /** __OCI_VCN_H_ */
