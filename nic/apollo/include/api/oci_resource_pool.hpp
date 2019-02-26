//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines resource pool API
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_OCI_RESOURCE_POOL_HPP__
#define __INCLUDE_API_OCI_RESOURCE_POOL_HPP__

#include "nic/apollo/include/api/oci.hpp"

/// \defgroup OCI_RESOURCE_POOL Resource pool API
/// @{

/// \brief Resource pool key
typedef struct oci_rsrc_pool_key_s {
    oci_rsrc_pool_id_t id;    ///< Resource pool ID
} __PACK__ oci_rsrc_pool_key_t;


/// \brief Traffic class type
typedef enum oci_traffic_class_type_s {
    OCI_TRAFFIC_CLASS_AGGR_TX    = 0,    ///< all traffic from vnic
    OCI_TRAFFIC_CLASS_AGGR_RX    = 1,    ///< all traffic to vnic
    OCI_TRAFFIC_CLASS_IGW_TX     = 2,    ///< traffic to internet gateway
    OCI_TRAFFIC_CLASS_IGW_RX     = 3,    ///< traffic from internet gateway
    OCI_TRAFFIC_CLASS_INT_SVC_TX = 4,    ///< traffic to local public service
    OCI_TRAFFIC_CLASS_INT_SVC_RX = 5,    ///< traffic from local public service
    OCI_TRAFFIC_CLASS_MAX        = 6,
} oci_traffic_class_type_t;

/// \brief Class limits
typedef struct oci_class_limits_s {
    uint32_t bytes_per_second;    ///< bps limit for given traffic class
} __PACK__ oci_class_limits_t;

/// \brief Connection tracking config
typedef struct oci_conntrack_cfg_s {
    uint32_t session_limit;    ///< Uppoer limit; 0 indicates unlimited sessions
} __PACK__ oci_conntrack_cfg_t;

/// \brief Resource limits
typedef struct oci_rsrc_limits_s {
    ///< Limits per class
    oci_class_limits_t class_limits[OCI_TRAFFIC_CLASS_MAX];
    ///< Connection tracking limits
    oci_conntrack_cfg_t conn_track;
} __PACK__ oci_rsrc_limits_t;

///< Resource pool
typedef struct oci_rsrc_pool_s {
    oci_rsrc_pool_id_t key;      ///< Key
    oci_rsrc_limits_t limits;    ///< Limits
} __PACK__ oci_rsrc_pool_t;

/// \brief Create resource pool
///
/// \param[in] rsrc_pool Resource pool information
///
/// \return #SDK_RET_OK on success, failure status code on error
oci_status_t oci_rsrc_pool_create(oci_rsrc_pool_t *rsrc_pool);

/// \brief Delete resource pool
///
/// \param[in] key Key
///
/// \return #SDK_RET_OK on success, failure status code on error
oci_status_t oci_rsrc_pool_delete(oci_rsrc_pool_key_t *key);

/// \@}

#endif    // __INCLUDE_API_OCI_RESOURCE_POOL_HPP_
