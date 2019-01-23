/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci_rsrc_pool.h
 *
 * @brief   This module defines OCI resource pool interface
 */

#if !defined (__OCI_RSRC_POOL_H_)
#define __OCI_RSRC_POOL_H_

#include "nic/apollo/include/api/oci.hpp"

/**
 * @defgroup OCI_RSRC_POOL - Resource pool specific API definitions
 *
 * @{
 */

/**
 * @brief Resource pool key
 */
typedef struct oci_rsrc_pool_key_s {
    oci_rsrc_pool_id_t id;    /**< Resource pool ID */
} __PACK__ oci_rsrc_pool_key_t;


/**
 * @brief Traffic class type
 */
typedef enum oci_traffic_class_type_s {
    OCI_TRAFFIC_CLASS_AGGR_TX    = 0,    /**< all traffic from vnic */
    OCI_TRAFFIC_CLASS_AGGR_RX    = 1,    /**< all traffic to vnic */
    OCI_TRAFFIC_CLASS_IGW_TX     = 2,    /**< traffic to internet gateway */
    OCI_TRAFFIC_CLASS_IGW_RX     = 3,    /**< traffic from internet gateway */
    OCI_TRAFFIC_CLASS_INT_SVC_TX = 4,    /**< traffic to local public service */
    OCI_TRAFFIC_CLASS_INT_SVC_RX = 5,    /**< traffic from local public service */
    OCI_TRAFFIC_CLASS_MAX        = 6,
} oci_traffic_class_type_t;

/**
 * @brief Class limits
 */
typedef struct oci_class_limits_s {
    uint32_t bytes_per_second;    /**< bps limit for given traffic class */
} __PACK__ oci_class_limits_t;

/**
 * @beirf Connection tracking config
 */
typedef struct oci_conntrack_cfg_s {
    uint32_t session_limit;    /**< upper limit to number of sessions,
                                    0 => unlimited */
} __PACK__ oci_conntrack_cfg_t;

/*
 * @brief Resource limits
 */
typedef struct oci_rsrc_limits_s {
    /**< Limits per traffic class */
    oci_class_limits_t class_limits[OCI_TRAFFIC_CLASS_MAX];
    oci_conntrack_cfg_t conn_track;    /**< connection tracking liimts */
} __PACK__ oci_rsrc_limits_t;

/**
 * @brief Resource pool
 */
typedef struct oci_rsrc_pool_s {
    oci_rsrc_pool_id_t key;      /**< id of the pool */
    oci_rsrc_limits_t limits;    /**< resource limts for this pool */
} __PACK__ oci_rsrc_pool_t;

/**
 * @brief Create Resource pool
 *
 * @param[in] rsrc_pool Resource pool information
 *
 * @return #SDK_RET_OK on success, failure status code on error
 */
oci_status_t oci_rsrc_pool_create(
    _In_ oci_rsrc_pool_t *rsrc_pool);

/**
 * @brief Delete Resource pool
 *
 * @param[in] rsrc_pool_key Resource pool key
 *
 * @return #SDK_RET_OK on success, failure status code on error
 */
oci_status_t oci_rsrc_pool_delete(
    _In_ oci_rsrc_pool_key_t *rsrc_pool_key);

/**
 * @}
 */
#endif /** __OCI_RSRC_POOL_H_ */
