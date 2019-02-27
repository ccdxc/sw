//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
///----------------------------------------------------------------------------
///
/// \file
/// This module defines SwitchPort API
///
///----------------------------------------------------------------------------

#ifndef __INCLUDE_API_SWITCHPORT_HPP__
#define __INCLUDE_API_SWITCHPORT_HPP__

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/apollo/p4/include/defines.h"
#include "nic/apollo/include/api/pds.hpp"

/// \defgroup PDS_SWITCHPORT Switchport API
/// @{

/// \brief Switchport specification
typedef struct pds_switchport_s {
    ipv4_addr_t switch_ip_addr;     ///< Switchport IP address
    mac_addr_t  switch_mac_addr;    ///< Switchport Mac Address
    ipv4_addr_t gateway_ip_addr;    ///< Gateway IP Address
} __PACK__ pds_switchport_spec_t;

/// \brief Switchport status
typedef struct pds_switchport_status_s {
    // TODO
} __PACK__ pds_switchport_status_t;

/// \brief Ingress drop statistics
typedef struct pds_switchport_idrop_stats_s {
    uint64_t drop_stats_pkts[P4I_DROP_REASON_MAX];
} __PACK__ pds_switchport_idrop_stats_t;

/// \brief Switchport egress drop statistics
typedef struct pds_switchport_edrop_stats_s {
    uint64_t drop_stats_pkts[0];
} __PACK__ pds_switchport_edrop_stats_t;

/// \brief Switchport statistics
typedef struct pds_switchport_stats_s {
    pds_switchport_idrop_stats_t idrop_stats;    ///<< Ingress drop statistics
    pds_switchport_edrop_stats_t edrop_stats;    ///<< Egress drop statistics
} __PACK__ pds_switchport_stats_t;

/// \brief Switchport information
typedef struct pds_switchport_info_s {
    pds_switchport_spec_t   spec;      ///< Specification
    pds_switchport_status_t status;    ///< Status
    pds_switchport_stats_t  stats;     ///< Statistics
} __PACK__ pds_switchport_info_t;

/// \brief Create switchport
///
/// \param[in] spec Specification
///
/// \return #SDK_RET_OK on success, failure status code on error
///
/// \remark
///  - switchport is a global configuration and can be created only once.
///  - Any other validation that is expected on the TEP should be done
///    by the caller
sdk_ret_t pds_switchport_create(pds_switchport_spec_t *spec);

/// \brief Read switchport
///
/// \param[out] info Information
///
/// \return #SDK_RET_OK on success, failure status code on error
///
sdk_ret_t pds_switchport_read(pds_switchport_info_t *info);

/// \brief Update switchport
///
/// \param[in] spec Specification
///
/// \return #SDK_RET_OK on success, failure status code on error
///
/// \remark
///  - A valid switchport specification should be passed
sdk_ret_t pds_switchport_update(pds_switchport_spec_t *spec);

/// \brief Delete switchport
///
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_switchport_delete(void);

/// \@}

#endif    // __INCLUDE_API_PDS_SWITCHPORT_HPP__
