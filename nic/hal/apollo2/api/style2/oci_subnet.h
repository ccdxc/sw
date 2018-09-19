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
 * @brief Attribute data for #OCI_SUBNET_ATTR_OPER_STATUS
 */
typedef enum _oci_subnet_oper_status_t
{
    /** Unknown */
    OCI_SUBNET_OPER_STATUS_UNKNOWN,

    /** Up */
    OCI_SUBNET_OPER_STATUS_UP,

    /** Down */
    OCI_SUBNET_OPER_STATUS_DOWN,

} oci_subnet_oper_status_t;

/**
 * @brief Attribute Id in oci_subnet_create(), oci_subnet_attribute_set() and
 * oci_subnet_attribute_get() calls
 */
typedef enum _oci_subnet_attr_t
{
    /**
     * @brief Start of attributes
     */
    OCI_SUBNET_ATTR_START,

    /* READ-ONLY */

    /**
     * @brief Operational Status
     *
     * @type oci_subnet_oper_status_t
     * @flags READ_ONLY
     */
    OCI_SUBNET_ATTR_OPER_STATUS = OCI_SUBNET_ATTR_START,

    /* READ-WRITE */

    /**
     * @brief VCN object ID
     *
     * @type oci_object_id_t
     * @flags MANDATORY_ON_CREATE | CREATE_ONLY | KEY
     * @objects OCI_OBJECT_TYPE_VCN
     */
    OCI_SUBNET_ATTR_VCN_ID,

    /**
     * @brief Subnet ID
     *
     * @type oci_subnet_id_t
     * @flags MANDATORY_ON_CREATE | CREATE_ONLY | KEY
     * @default wildcard
     */
    OCI_SUBNET_ATTR_ID,

    /**
     * @brief CIDR block
     *
     * @type oci_ip_prefix_t
     * @flags CREATE_AND_SET
     * @default wildcard
     */
    OCI_SUBNET_ATTR_CIDR_BLOCK,

    /**
     * @brief End of attributes
     */
    OCI_SUBNET_ATTR_END,

} oci_subnet_attr_t;

/**
 * @brief Create Subnet
 *
 * @param[out] subnet_id Subnet ID 
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_subnet_create (
        _Out_ oci_object_id_t *subnet_id,
        _In_ uint32_t attr_count,
        _In_ const oci_attribute_t *attr_list);

/**
 * @brief Set Subnet attribute value.
 *
 * @param[in] subnet_id Subnet ID 
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_subnet_attribute_set (
        _In_ oci_object_id_t subnet_id,
        _In_ uint32_t attr_count,
        _In_ const oci_attribute_t *attr_list);

/**
 * @brief Get Subnet attribute value.
 *
 * @param[in] subnet_id Subnet ID 
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_subnet_attribute_get (
        _In_ oci_object_id_t subnet_id,
        _In_ uint32_t attr_count,
        _Inout_ oci_attribute_t *attr_list);

/**
 * @brief Delete Subnet
 *
 * @param[in] subnet_id Subnet ID 
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_delete_subnet_delete (
        _In_ oci_object_id_t subnet_id);

/**
 * @}
 */
#endif /** __OCI_SUBNET_H_ */
