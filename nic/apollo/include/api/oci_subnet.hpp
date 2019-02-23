//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines subnet API
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_SUBNET_HPP__
#define __INCLUDE_API_SUBNET_HPP__

#include "nic/sdk/include/sdk/types.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/include/api/oci.hpp"
#include "nic/apollo/include/api/oci_vcn.hpp"
#include "nic/apollo/include/api/oci_policy.hpp"
#include "nic/apollo/include/api/oci_route.hpp"

/// \defgroup OCI_SUBNET Subnet
/// @{

// TODO: we have to confirm this number !!!
#define OCI_MAX_SUBNET (5 * OCI_MAX_VCN)

/// \brief Key
typedef struct oci_subnet_key_s {
    oci_subnet_id_t id;    ///< Subnet id
} __PACK__ oci_subnet_key_t;

/// \brief Spec
typedef struct oci_subnet_spec_s {
    oci_subnet_key_t key;                    ///< Key
    oci_vcn_key_t vcn;                       ///< VCN key
    ip_prefix_t pfx;                         ///< CIDR block
    ip_addr_t vr_ip;                         ///< Virtual router IP
    mac_addr_t vr_mac;                       ///< Virtual router mac
    oci_route_table_key_t v4_route_table;    ///< Route table id
    oci_route_table_key_t v6_route_table;    ///< Route table id
    oci_policy_key_t ing_v4_policy;          ///< ingress IPv4 policy table
    oci_policy_key_t ing_v6_policy;          ///< ingress IPv6 policy table
    oci_policy_key_t egr_v4_policy;          ///< egress IPv4 policy table
    oci_policy_key_t egr_v6_policy;          ///< egress IPv6 policy table
} __PACK__ oci_subnet_spec_t;

/// \brief Status
typedef struct oci_subnet_status_s {
    uint16_t hw_id;
    mem_addr_t policy_base_addr;
} __PACK__ oci_subnet_status_t;

/// \brief Statistics
typedef struct oci_subnet_stats_s {
    // TODO
} __PACK__ oci_subnet_stats_t;

/// \brief Info
typedef struct oci_subnet_info_s {
    oci_subnet_spec_t spec;
    oci_subnet_status_t status;
    oci_subnet_stats_t stats;
} __PACK__ oci_subnet_info_t;

/// \brief Create subnet
///
/// \param[in] subnet Subnet specification
///
/// \return #SDK_RET_OK on success, failure status code on error
///
/// \remark
///  - A valid VCN id should be used
///  - Subnet prefix passed should be valid as per VCN prefix
///  - Subnet prefix should not overlap with any other subnet
///  - Subnet with same id should not be created again
///  - Any other validation that is expected on the subnet should be done
///    by the caller
sdk_ret_t oci_subnet_create(oci_subnet_spec_t *subnet);

/// \brief Read subnet
///
/// \param[in/out] subnet_spec Spec
///
/// \return #SDK_RET_OK on success, failure status code on error
///
/// \remark
///  - Subnet spec containing a valid subnet key should be passed
sdk_ret_t oci_subnet_read(oci_subnet_key_t *key, oci_subnet_info_t *info);

/// \brief Update subnet
///
/// \param[in] subnet_spec Spec
///
/// \return #SDK_RET_OK on success, failure status code on error
///
/// \remark
///  - A valid subnet spec should be passed
sdk_ret_t oci_subnet_update(oci_subnet_spec_t *subnet_spec);

/// \brief Delete subnet
///
/// \param[in] subnet_key Key
///
/// \return #SDK_RET_OK on success, failure status code on error
///
/// \remark
///  - A valid subnet key should be passed
sdk_ret_t oci_subnet_delete(oci_subnet_key_t *subnet_key);

/// \@}

#endif    // __INCLUDE_API_SUBNET_HPP__
