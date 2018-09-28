/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci_resource_pool.h
 *
 * @brief   This module defines OCI resource pool interface
 */

#if !defined (__OCI_RESOURCE_POOL_H_)
#define __OCI_RESOURCE_POOL_H_

#include <oci_types.h>

/**
 * @defgroup OCI_RESOURCE_POOL - Resource pool specific API definitions
 *
 * @{
 */

/**
 * @brief resource pool key
 */
typedef struct _oci_resource_pool_key_t
{
    oci_resource_pool_id_t id;    /**< Resource pool ID */

} PACKED oci_resource_pool_key_t;

typedef enum _oci_traffic_class_type_t {
    OCI_TRAFFIC_CLASS_NONE            = 0,
    OCI_TRAFFIC_CLASS_AGGR_TX         = 1,    /**< all traffic from vnic */
    OCI_TRAFFIC_CLASS_AGGR_RX         = 2,    /**< all traffic to vnic */
    OCI_TRAFFIC_CLASS_IGW_TX          = 3,    /**< traffic going to Internet gateway */
    OCI_TRAFFIC_CLASS_IGW_RX          = 4,    /**< traffic coming from Internet gateway */
    OCI_TRAFFIC_CLASS_INTERNAL_SVC_TX = 5,    /**< traffic going to local public service */
    OCI_TRAFFIC_CLASS_INTERNAL_SVC_RX = 5,    /**< traffic coming from local public service */
    OCI_TRAFFIC_CLASS_MAX             = 6,
} oci_traffic_class_type_t;

typedef struct _oci_class_limits_t {
    uint32_t    bytes_per_second;    /**< bps limit for given traffic class */
} PACKED oci_class_limits_t;

typedef struct _oci_conntrack_cfg_t {
    uint32_t session_limit;    /**< upper limit to number of sessions, 0 => unlimited */
} PACKED oci_conntrack_cfg_t;

typedef struct _oci_resource_limits_t {
    oci_class_limits_t     class_limits[OCI_TRAFFIC_CLASS_MAX]; /**< limts per traffic class */
    oci_conntrack_cfg_t    conn_track;                          /**< connection tracking liimts */
} PACKED oci_resource_limits_t;

typedef struct _oci_resource_pool_t
{
    oci_resource_pool_id_t key;       /**< id of the pool */
    oci_resource_limits_t  limits;    /**< resource limts for this pool */
} PACKED oci_resource_pool_t;

/**
 * @brief Create Resource pool
 *
 * @param[in] resource_pool Resource pool information
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_resource_pool_create(
                 _In_ oci_resource_pool_t *resource_pool);


/**
 * @brief Delete Resource pool
 *
 * @param[in] resource_pool_key Resource pool key
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_resource_pool_delete(
                 _In_ oci_resource_pool_key_t *resource_pool_key);

/**
 * @}
 */
#endif /** __OCI_RESOURCE_POOL_H_ */
