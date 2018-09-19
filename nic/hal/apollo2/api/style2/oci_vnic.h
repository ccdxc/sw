/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci_vnic.h
 *
 * @brief   This module defines OCI VNIC interface
 */

#if !defined (__OCI_VNIC_H_)
#define __OCI_VNIC_H_

#include <oci_types.h>

/**
 * @defgroup OCI_VNIC - VNIC specific API definitions
 *
 * @{
 */

/**
 * @brief Attribute data for #OCI_VNIC_ATTR_OPER_STATUS
 */
typedef enum _oci_vnic_oper_status_t
{
    /** Unknown */
    OCI_VNIC_OPER_STATUS_UNKNOWN,

    /** Up */
    OCI_VNIC_OPER_STATUS_UP,

    /** Down */
    OCI_VNIC_OPER_STATUS_DOWN,

    /** Test Running */
    OCI_VNIC_OPER_STATUS_TESTING,

} oci_vnic_oper_status_t;

/**
 * @brief Attribute Id in oci_vnic_create(), oci_vnic_attribute_set() and
 * oci_vnic_attribute_get() calls
 */
typedef enum _oci_vnic_attr_t
{
    /**
     * @brief Start of attributes
     */
    OCI_VNIC_ATTR_START,

    /* READ-ONLY */

    /**
     * @brief Operational Status
     *
     * @type oci_vnic_oper_status_t
     * @flags READ_ONLY
     */
    OCI_VNIC_ATTR_OPER_STATUS = OCI_VNIC_ATTR_START,

    /* READ-WRITE */

    /**
     * @brief VCN object ID
     *
     * @type oci_object_id_t
     * @flags MANDATORY_ON_CREATE | CREATE_ONLY | KEY
     * @objects OCI_OBJECT_TYPE_VCN
     */
    OCI_VNIC_ATTR_VCN_ID,

    /**
     * @brief Subnet object ID
     *
     * @type oci_object_id_t
     * @flags MANDATORY_ON_CREATE | CREATE_ONLY | KEY
     * @objects OCI_OBJECT_TYPE_SUBNET
     */
    OCI_VNIC_ATTR_SUBNET_ID,

    /**
     * @brief VNIC ID
     *
     * @type oci_vnic_id_t
     * @flags MANDATORY_ON_CREATE | CREATE_ONLY | KEY
     */
    OCI_VNIC_ATTR_ID,

    /**
     * @brief VLAN ID 
     *
     * @type oci_vlan_id_t
     * @flags MANDATORY_ON_CREATE | CREATE_ONLY
     */
    OCI_VNIC_ATTR_VLAN_ID,

    /**
     * @brief MPLS Tag 
     *
     * @type oci_mpls_tag_t
     * @flags MANDATORY_ON_CREATE | CREATE_ONLY
     */
    OCI_VNIC_ATTR_MPLS_TAG,

    /**
     * @brief MAC Address
     *
     * @type oci_mac_addr_t
     * @flags MANDATORY_ON_CREATE | CREATE_ONLY
     */
    OCI_VNIC_ATTR_MAC,

    /**
     * @brief Primary IP Address
     *
     * @type oci_ip_addr_t
     * @flags MANDATORY_ON_CREATE | CREATE_ONLY
     */
    OCI_VNIC_ATTR_PRIMARY_IP,

    /**
     * @brief Primary Substrate IP Address
     *
     * @type oci_ip_addr_t
     * @flags MANDATORY_ON_CREATE | CREATE_ONLY
     */
    OCI_VNIC_ATTR_PRIMARY_SUBSTRATE_IP,

    /**
     * @brief Is src/dst check enabled?
     *
     * @type bool
     * @flags CREATE_AND_SET
     */
    OCI_VNIC_ATTR_SRC_DST_CHECK,

    /**
     * @brief End of attributes
     */
    OCI_VNIC_ATTR_END,

} oci_vnic_attr_t;

/**
 * @brief Create VNIC
 *
 * @param[out] vnic_id VNI id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_vnic_create (
        _Out_ oci_object_id_t *vnic_id,
        _In_ uint32_t attr_count,
        _In_ const oci_attribute_t *attr_list);

/**
 * @brief Set VNIC attribute value.
 *
 * @param[in] vnic_key VNIC key
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_vnic_attribute_set (
        _In_ oci_object_id_t vnic_id,
        _In_ uint32_t attr_count,
        _In_ const oci_attribute_t *attr_list);

/**
 * @brief Get VNIC attribute value.
 *
 * @param[in] vnic_id VNIC id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_vnic_attribute_get (
        _In_ oci_object_id_t vnic_id,
        _In_ uint32_t attr_count,
        _Inout_ oci_attribute_t *attr_list);

/**
 * @brief Delete VNIC
 *
 * @param[in] vnic_id VNIC id
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_vnic_delete (
        _In_ oci_object_id_t vnic_id);

/**
 * @}
 */
#endif /** __OCI_VNIC_H_ */
