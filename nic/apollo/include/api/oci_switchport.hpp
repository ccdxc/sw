//
//// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
////
////----------------------------------------------------------------------------
/////
///// \file
///// This module defines SwitchPort API
/////
////----------------------------------------------------------------------------

#ifndef __INCLUDE_API_SWITCHPORT_HPP__
#define __INCLUDE_API_SWITCHPORT_HPP__

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/apollo/p4/include/defines.h"
#include "nic/apollo/include/api/oci.hpp"

/// \defgroup OCI_SWITCHPORT switchport
/// @{

/// \brief switchport specification
typedef struct oci_switchport_s {
    ipv4_addr_t switch_ip_addr;  /// SWITCHPORT IP Address
    mac_addr_t  switch_mac_addr; /// SWITCHPORT Mac Address
    ipv4_addr_t gateway_ip_addr; /// Gateway IP Address
} __PACK__ oci_switchport_spec_t;

/// \brief switchport status
typedef struct oci_switchport_status_s {
    // TODO
} __PACK__ oci_switchport_status_t;

/// \brief ingress drop statistics
typedef struct oci_switchport_idrop_stats_s {
	uint64_t drop_stats_pkts[P4I_DROP_REASON_MAX];
} __PACK__ oci_switchport_idrop_stats_t;

/// \brief ingress drop statistics
typedef struct oci_switchport_edrop_stats_s {
	uint64_t drop_stats_pkts[0];
} __PACK__ oci_switchport_edrop_stats_t;

/// \brief switchport statistics
typedef struct oci_switchport_stats_s {
    // TODO
    oci_switchport_idrop_stats_t ingress_drop_stats;
    oci_switchport_edrop_stats_t egress_drop_stats;
} __PACK__ oci_switchport_stats_t;

typedef struct oci_switchport_info_s {
    oci_switchport_spec_t   spec;
    oci_switchport_status_t status;
    oci_switchport_stats_t  stats;
} __PACK__ oci_switchport_info_t;

/// \brief Create switchport
///
/// \param[in] spec switchport specification
///
/// \return #SDK_RET_OK on success, failure status code on error
///
/// \remark
///  - switchport is a global configuration and can be created only once.
///  - Any other validation that is expected on the TEP should be done
///    by the caller
sdk_ret_t oci_switchport_create(oci_switchport_spec_t *spec);

/// \brief Read switchport
///
/// \param[out] spec switchport specification
/// \param[out] status switchport status
/// \param[out] stats switchport statistics
///
/// \return #SDK_RET_OK on success, failure status code on error
///
sdk_ret_t oci_switchport_read(oci_switchport_info_t *info);

/// \brief Update switchport
///
/// \param[in] spec switchport specification
///
/// \return #SDK_RET_OK on success, failure status code on error
///
/// \remark
///  - A valid switchport specification should be passed
sdk_ret_t oci_switchport_update(oci_switchport_spec_t *spec);

/// \brief Delete switchport
///
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t oci_switchport_delete(void);

/// \@}

#endif    // __INCLUDE_API_SWITCHPORT_HPP__
