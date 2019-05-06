//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
///----------------------------------------------------------------------------
///
/// \file
/// This module defines Device API
///
///----------------------------------------------------------------------------

#ifndef __INCLUDE_API_DEVICE_HPP__
#define __INCLUDE_API_DEVICE_HPP__

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/apollo/p4/include/defines.h"
#include "nic/apollo/api/include/pds.hpp"

/// \defgroup PDS_DEVICE Device API
/// @{

/// \brief Device specification
typedef struct pds_device_s {
    ipv4_addr_t device_ip_addr;     ///< Device IP address
    mac_addr_t  device_mac_addr;    ///< Device Mac Address
    ipv4_addr_t gateway_ip_addr;    ///< Gateway IP Address
} __PACK__ pds_device_spec_t;

/// \brief Device status
typedef struct pds_device_status_s {
    // TODO
} __PACK__ pds_device_status_t;

/// \brief Ingress drop statistics
typedef struct pds_device_ing_drop_stats_s {
    uint64_t drop_stats_pkts[P4I_DROP_REASON_MAX + 1];
} __PACK__ pds_device_ing_drop_stats_t;

/// \brief Device egress drop statistics
typedef struct pds_device_egr_drop_stats_s {
    uint64_t drop_stats_pkts[0];
} __PACK__ pds_device_egr_drop_stats_t;

/// \brief Device statistics
typedef struct pds_device_stats_s {
    pds_device_ing_drop_stats_t ing_drop_stats;    ///<< Ingress drop statistics
    pds_device_egr_drop_stats_t egr_drop_stats;    ///<< Egress drop statistics
} __PACK__ pds_device_stats_t;

/// \brief Device information
typedef struct pds_device_info_s {
    pds_device_spec_t   spec;      ///< Specification
    pds_device_status_t status;    ///< Status
    pds_device_stats_t  stats;     ///< Statistics
} __PACK__ pds_device_info_t;

/// \brief Create device
///
/// \param[in] spec Specification
/// \return #SDK_RET_OK on success, failure status code on error
///
/// \remark
///  - device is a global configuration and can be created only once.
///  - Any other validation that is expected on the TEP should be done
///    by the caller
sdk_ret_t pds_device_create(pds_device_spec_t *spec);

/// \brief Read device
///
/// \param[out] info Information
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_device_read(pds_device_info_t *info);

/// \brief Update device
///
/// \param[in] spec Specification
/// \return #SDK_RET_OK on success, failure status code on error
///
/// \remark
///  - A valid device specification should be passed
sdk_ret_t pds_device_update(pds_device_spec_t *spec);

/// \brief Delete device
///
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_device_delete(void);

/// \@}

#endif    // __INCLUDE_API_PDS_DEVICE_HPP__
