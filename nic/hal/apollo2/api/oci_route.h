/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci_route.h
 *
 * @brief   This module defines OCI Route interface
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
 * @brief Route key
 */
typedef struct _oci_route_key_t
{
    oci_ip_prefix_t dst_pfx;    /**< IP prefix destination */

} oci_route_key_t;

/**
 * @brief Route
 */
typedef struct _oci_route_t
{
    oci_route_key_t key;       /**< Route key */
    oci_ip_addr_t nh_ip;    /**< Next hop IP address */

} oci_route_t;

/**
 * @brief Route list
 */
typedef struct _oci_route_list_t
{
    oci_uint32_t count;           /**< Number of routes in the list */
    oci_route_t route_list[0];    /**< List or routes */

} oci_route_list_t;

/**
 * @brief Route Group key
 */
typedef struct _oci_route_grp_key_t
{
    oci_subnet_id_t subnet_id;    /**< Subnet ID */

} PACKED oci_route_grp_key_t;

/**
 * @brief Route Group
 */
typedef struct _oci_route_grp_t
{
    oci_route_grp_key_t key;        /**< Route group key */
    oci_route_list_t route_list;    /**< List of routes in the group */

} PACKED oci_route_grp_t;

/**
 * @brief Create route group 
 *
 * @param[in] route_grp Route Group information
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_route_grp_create (
        _In_ oci_route_grp_t *route_grp);


/**
 * @brief Delete route group
 *
 * @param[in] route_grp_key Route Group Key
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_route_grp_delete (
        _In_ oci_route_grp_key_t *route_grp_key);

/**
 * @}
 */
#endif /** __OCI_ROUTE_H_ */
