/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci_rt.h
 *
 * @brief   This module defines OCI Route interface
 */

#if !defined (__OCI_RT_H_)
#define __OCI_RT_H_

#include <oci_types.h>

/**
 * @defgroup OCI_RT - Route specific API definitions
 *
 * @{
 */

/**
 * @brief Route key
 */
typedef struct _oci_rt_key_t
{
    /**
     * @brief IP Prefix Destination
     */
    oci_ip_prefix_t dst_pfx;

} oci_rt_key_t;

/**
 * @brief Route
 */
typedef struct _oci_rt_t
{
    /**
     * @brief Route key
     */
    oci_rt_key_t key;

    /**
     * @brief Next hop IP Address
     */
    oci_ip_addr_t nh_ip;

} oci_rt_t;

/**
 * @brief Route list
 */
typedef struct _oci_rt_list_t
{
    /**
     * @brief Number of routes in the list
     */
    oci_uint32_t count;

    /**
     * @brief List or routes
     */
    oci_rt_t rt_list[0];

} oci_rt_list_t;

/**
 * @brief Route Group key
 */
typedef struct _oci_rt_grp_key_t
{
    /**
     * @brief Subnet ID
     */
    oci_subnet_id_t subnet_id;

} PACKED oci_rt_grp_key_t;

/**
 * @brief Route Group
 */
typedef struct _oci_rt_grp_t
{
    /**
     * @brief Route Group key
     */
    oci_rt_grp_key_t key;

    /**
     * @brief List of routes in the group
     */
    oci_rt_list_t rt_list;

} PACKED oci_rt_grp_t;

/**
 * @brief Create route group 
 *
 * @param[in] rt_grp Route Group information
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_rt_grp_create (
        _In_ oci_rt_grp_t *rt_grp);


/**
 * @brief Delete route group
 *
 * @param[in] rt_grp_key Route Group Key
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_rt_grp_delete (
        _In_ oci_rt_grp_key_t *rt_grp_key);

/**
 * @}
 */
#endif /** __OCI_RT_H_ */
