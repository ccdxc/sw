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
#include "nic/apollo/api/include/pds.hpp"

#define PDS_DROP_REASON_MAX        64
#define PDS_MAX_DROP_NAME_LEN      32

/// \defgroup PDS_DEVICE Device API
/// \@{

/// \brief device specification
typedef struct pds_device_s {
    ipv4_addr_t device_ip_addr;     ///< device IP address
    mac_addr_t  device_mac_addr;    ///< device MAC address
    ipv4_addr_t gateway_ip_addr;    ///< gateway IP address
} __PACK__ pds_device_spec_t;

/// \brief device status
typedef struct pds_device_status_s {
    // TODO
} __PACK__ pds_device_status_t;

/// \brief Drop statistics
typedef struct pds_device_drop_stats_s {
    char     name[PDS_MAX_DROP_NAME_LEN];
    uint64_t count;
} __PACK__ pds_device_drop_stats_t;

/// \brief device statistics
typedef struct pds_device_stats_s {
    uint32_t ing_drop_stats_count;    ///<< Number of entries in the ingress drop statistics
    uint32_t egr_drop_stats_count;    ///<< Number of entries in the egress drop statistics
    pds_device_drop_stats_t ing_drop_stats[PDS_DROP_REASON_MAX];    ///<< Ingress drop statistics
    pds_device_drop_stats_t egr_drop_stats[PDS_DROP_REASON_MAX];    ///<< Egress drop statistics
} __PACK__ pds_device_stats_t;

/// \brief device information
typedef struct pds_device_info_s {
    pds_device_spec_t   spec;      ///< specification
    pds_device_status_t status;    ///< status
    pds_device_stats_t  stats;     ///< statistics
} __PACK__ pds_device_info_t;

/// \brief     create device
/// \param[in] spec specification
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    device is a global configuration and can be created only once.
///            Any other validation that is expected on the TEP should be done
///            by the caller
sdk_ret_t pds_device_create(pds_device_spec_t *spec);

/// \brief      read device
/// \param[out] info device information
/// \return     #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_device_read(pds_device_info_t *info);

/// \brief     update device
/// \param[in] spec specification
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid device specification should be passed
sdk_ret_t pds_device_update(pds_device_spec_t *spec);

/// \brief  delete device
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_device_delete(void);

/// \@}

#endif    // __INCLUDE_API_PDS_DEVICE_HPP__
