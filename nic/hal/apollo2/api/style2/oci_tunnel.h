/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file   oci_tunnel.h
 *
 * @brief   This module defines OCI tunnel interface
 */

#if !defined (__OCI_TUNNEL_H_)
#define __OCI_TUNNEL_H_

#include <oci_types.h>

/**
 * @defgroup OCI_TUNNEL- Tunnel API definitions
 *
 * @{
 */

/**
 * @brief Attribute data for #OCI_TUNNEL_ATTR_OPER_STATUS
 */
typedef enum _oci_tunnel_oper_status_t
{
    /** Unknown */
    OCI_TUNNEL_OPER_STATUS_UNKNOWN,

    /** Up */
    OCI_TUNNEL_OPER_STATUS_UP,

    /** Down */
    OCI_TUNNEL_OPER_STATUS_DOWN,

} oci_tunnel_oper_status_t;

/**
 * @brief Defines tunnel type
 */
typedef enum _oci_tunnel_type_t
{
    OCI_TUNNEL_TYPE_IPINIP_GRE,

} oci_tunnel_type_t;

/**
 * @brief Attribute Id in oci_tunnel_create(), oci_tunnel_attribute_set() and
 * oci_tunnel_attribute_get() calls
 */
typedef enum _oci_tunnel_attr_t
{
    /**
     * @brief Start of attributes
     */
    OCI_TUNNEL_ATTR_START,

    /* READ-ONLY */

    /**
     * @brief Operational Status
     *
     * @type oci_tunnel_oper_status_t
     * @flags READ_ONLY
     */
    OCI_TUNNEL_ATTR_OPER_STATUS = OCI_TUNNEL_ATTR_START,

    /* READ-WRITE */

    /**
     * @brief Tunnel type
     *
     * @type oci_tunnel_type_t
     * @flags MANDATORY_ON_CREATE | CREATE_ONLY | KEY
     */
    OCI_TUNNEL_ATTR_TYPE,

    /**
     * @brief Tunnel destination IP address
     *
     * @type oci_ip_prefix_t
     * @flags MANDATORY_ON_CREATE | CREATE_ONLY | KEY
     */
    OCI_TUNNEL_ATTR_DST_IP,

    /**
     * @brief End of attributes
     */
    OCI_TUNNEL_ATTR_END,

} oci_tunnel_attr_t;

/**
 * @brief Create tunnel
 *
 * @param[out] tunnel_id Tunnel id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_tunnel_create (
        _Out_ oci_object_id_t *tunnel_id,
        _In_ uint32_t attr_count,
        _In_ const oci_attribute_t *attr_list);

/**
 * @brief Set tunnel attribute
 *
 * @param[in] tunnel_id Tunnel id
 * @param[in] attr Attribute
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_set_tunnel_attribute_set (
        _In_ oci_object_id_t tunnel_id,
        _In_ uint32_t attr_count,
        _Inout_ oci_attribute_t *attr_list);

/**
 * @brief Get tunnel attributes
 *
 * @param[in] tunnel_id Tunnel id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_tunnel_attribute_get (
        _In_ oci_object_id_t tunnel_id,
        _In_ uint32_t attr_count,
        _Inout_ oci_attribute_t *attr_list);

/**
 * @brief Delete tunnel
 *
 * @param[in] tunnel_id Tunnel id
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_unnel_delete (
        _In_ oci_object_id_t tunnel_id);

/**
 * @}
 */
#endif /** __OCI_TUNNEL_H_ */
