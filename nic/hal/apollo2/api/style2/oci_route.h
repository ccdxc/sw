/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci_route.h
 *
 * @brief   This module defines OCI Route Entry interface
 */

#if !defined (__OCI_ROUTE_H_)
#define __OCI_ROUTE_H_

#include <oci_types.h>

/**
 * @defgroup OCI_ROUTE - Route specific API definitions
 *
 * @{
 */

/**
 * @brief Attribute Id for OCI route object
 */
typedef enum _oci_route_entry_attr_t
{
    /**
     * @brief Start of attributes
     */
    OCI_ROUTE_ENTRY_ATTR_START,

    /* READ-WRITE */

    /**
     * @brief Next hop IP Address
     *
     * @type oci_ip_addr_t
     * @flags CREATE_AND_SET
     */
    OCI_ROUTE_ENTRY_ATTR_NEXT_HOP_IP = OCI_ROUTE_ENTRY_ATTR_START,

    /**
     * @brief End of attributes
     */
    OCI_ROUTE_ENTRY_ATTR_END,

} oci_route_entry_attr_t;

/**
 * @brief Unicast route entry
 */
typedef struct _oci_route_entry_t
{
    /**
     * @brief VCN ID
     *
     * @objects OCI_OBJECT_TYPE_VCN
     */
    oci_object_id_t vcn_id;

    /**
     * @brief Subnet ID
     *
     * @objects OCI_OBJECT_TYPE_SUBNET
     */
    oci_object_id_t subnet_id;

    /**
     * @brief IP Prefix Destination
     */
    oci_ip_prefix_t destination;

} oci_route_entry_t;

/**
 * @brief Create Route
 *
 * Note: IP prefix/mask expected in Network Byte Order.
 *
 * @param[in] route_entry Route entry
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_route_entry_create (
        _In_ const oci_route_entry_t *route_entry,
        _In_ uint32_t attr_count,
        _In_ const oci_attribute_t *attr_list);

/**
 * @brief Set route attribute value
 *
 * @param[in] route_entry Route entry
 * @param[in] attr Attribute
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_route_entry_attribute_set (
        _In_ const oci_route_entry_t *route_entry,
        _In_ const oci_attribute_t *attr);

/**
 * @brief Get route attribute value
 *
 * @param[in] route_entry Route entry
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_route_entry_attribute_get (
        _In_ const oci_route_entry_t *route_entry,
        _In_ uint32_t attr_count,
        _Inout_ oci_attribute_t *attr_list);

/**
 * @brief Delete Route
 *
 * Note: IP prefix/mask expected in Network Byte Order.
 *
 * @param[in] route_entry Route entry
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_route_entry_delete (
        _In_ const oci_route_entry_t *route_entry);

/**
 * @brief Bulk create route entry
 *
 * @param[in] object_count Number of objects to create
 * @param[in] route_entry List of object to create
 * @param[in] attr_count List of attr_count. Caller passes the number
 *    of attribute for each object to create.
 * @param[in] attr_list List of attributes for every object.
 * @param[in] mode Bulk operation error handling mode.
 * @param[out] object_statuses List of status for every object. Caller needs to
 * allocate the buffer
 *
 * @return #OCI_STATUS_SUCCESS on success when all objects are created or
 * #OCI_STATUS_FAILURE when any of the objects fails to create. When there is
 * failure, Caller is expected to go through the list of returned statuses to
 * find out which fails and which succeeds.
 */
oci_status_t oci_route_entry_bulk_create (
        _In_ uint32_t object_count,
        _In_ const oci_route_entry_t *route_entry,
        _In_ const uint32_t *attr_count,
        _In_ const oci_attribute_t **attr_list,
        _In_ oci_bulk_op_error_mode_t mode,
        _Out_ oci_status_t *object_statuses);

/**
 * @brief Bulk set attribute on route entry
 *
 * @param[in] object_count Number of objects to set attribute
 * @param[in] route_entry List of objects to set attribute
 * @param[in] attr_list List of attributes to set on objects, one attribute per object
 * @param[in] mode Bulk operation error handling mode.
 * @param[out] object_statuses List of status for every object. Caller needs to
 * allocate the buffer
 *
 * @return #OCI_STATUS_SUCCESS on success when all objects are removed or
 * #OCI_STATUS_FAILURE when any of the objects fails to remove. When there is
 * failure, Caller is expected to go through the list of returned statuses to
 * find out which fails and which succeeds.
 */
oci_status_t oci_route_entry_attribute_bulk_set (
        _In_ uint32_t object_count,
        _In_ const oci_route_entry_t *route_entry,
        _In_ const oci_attribute_t *attr_list,
        _In_ oci_bulk_op_error_mode_t mode,
        _Out_ oci_status_t *object_statuses);

/**
 * @brief Bulk get attribute on route entry
 *
 * @param[in] object_count Number of objects to set attribute
 * @param[in] route_entry List of objects to set attribute
 * @param[in] attr_count List of attr_count. Caller passes the number
 *    of attribute for each object to get
 * @param[inout] attr_list List of attributes to set on objects, one attribute per object
 * @param[in] mode Bulk operation error handling mode
 * @param[out] object_statuses List of status for every object. Caller needs to
 * allocate the buffer
 *
 * @return #OCI_STATUS_SUCCESS on success when all objects are removed or
 * #OCI_STATUS_FAILURE when any of the objects fails to remove. When there is
 * failure, Caller is expected to go through the list of returned statuses to
 * find out which fails and which succeeds.
 */
oci_status_t oci_route_entry_attribute_bulk_get (
        _In_ uint32_t object_count,
        _In_ const oci_route_entry_t *route_entry,
        _In_ const uint32_t *attr_count,
        _Inout_ oci_attribute_t **attr_list,
        _In_ oci_bulk_op_error_mode_t mode,
        _Out_ oci_status_t *object_statuses);

/**
 * @brief Bulk delete route entry
 *
 * @param[in] object_count Number of objects to remove
 * @param[in] route_entry List of objects to remove
 * @param[in] mode Bulk operation error handling mode.
 * @param[out] object_statuses List of status for every object. Caller needs to
 * allocate the buffer
 *
 * @return #OCI_STATUS_SUCCESS on success when all objects are removed or
 * #OCI_STATUS_FAILURE when any of the objects fails to remove. When there is
 * failure, Caller is expected to go through the list of returned statuses to
 * find out which fails and which succeeds.
 */
oci_status_t oci_route_entry_bulk_delete (
        _In_ uint32_t object_count,
        _In_ const oci_route_entry_t *route_entry,
        _In_ oci_bulk_op_error_mode_t mode,
        _Out_ oci_status_t *object_statuses);

/**
 * @}
 */
#endif /** __OCI_ROUTE_H_ */
