//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines subnet API
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_PDS_SUBNET_HPP__
#define __INCLUDE_API_PDS_SUBNET_HPP__

#include "nic/sdk/include/sdk/types.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/include/api/pds.hpp"
#include "nic/apollo/include/api/pds_vcn.hpp"
#include "nic/apollo/include/api/pds_policy.hpp"
#include "nic/apollo/include/api/pds_route.hpp"

/// \defgroup PDS_SUBNET Subnet API
/// @{

// TODO: we have to confirm this number !!!
#define PDS_MAX_SUBNET (5 * PDS_MAX_VCN)

/// \brief Subnet key
typedef struct pds_subnet_key_s {
    pds_subnet_id_t id;    ///< Subnet ID
} __PACK__ pds_subnet_key_t;

/// \brief Subnet specification
typedef struct pds_subnet_spec_s {
    pds_subnet_key_t key;                    ///< Key
    pds_vcn_key_t vcn;                       ///< VCN key
    ip_prefix_t pfx;                         ///< CIDR block
    ip_addr_t vr_ip;                         ///< Virtual router IP
    mac_addr_t vr_mac;                       ///< Virtual router mac
    pds_route_table_key_t v4_route_table;    ///< Route table id
    pds_route_table_key_t v6_route_table;    ///< Route table id
    pds_policy_key_t ing_v4_policy;          ///< ingress IPv4 policy table
    pds_policy_key_t ing_v6_policy;          ///< ingress IPv6 policy table
    pds_policy_key_t egr_v4_policy;          ///< egress IPv4 policy table
    pds_policy_key_t egr_v6_policy;          ///< egress IPv6 policy table
} __PACK__ pds_subnet_spec_t;

/// \brief Subnet status
typedef struct pds_subnet_status_s {
    uint16_t hw_id;                 ///< Hardware ID
    mem_addr_t policy_base_addr;    ///< Policy base address
} __PACK__ pds_subnet_status_t;

/// \brief Subnet statistics
typedef struct pds_subnet_stats_s {
    // TODO
} __PACK__ pds_subnet_stats_t;

/// \brief Subnet information
typedef struct pds_subnet_info_s {
    pds_subnet_spec_t spec;        ///< Specification
    pds_subnet_status_t status;    ///< Status
    pds_subnet_stats_t stats;      ///< Statistics
} __PACK__ pds_subnet_info_t;

/// \brief Create subnet
///
/// \param[in] spec Specification
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
sdk_ret_t pds_subnet_create(pds_subnet_spec_t *spec);

/// \brief Read subnet
///
/// \param[in] key Key
/// \param[out] info Information
///
/// \return #SDK_RET_OK on success, failure status code on error
///
/// \remark
///  - Subnet spec containing a valid subnet key should be passed
sdk_ret_t pds_subnet_read(pds_subnet_key_t *key, pds_subnet_info_t *info);

/// \brief Update subnet
///
/// \param[in] spec Specification
///
/// \return #SDK_RET_OK on success, failure status code on error
///
/// \remark
///  - A valid subnet spec should be passed
sdk_ret_t pds_subnet_update(pds_subnet_spec_t *spec);

/// \brief Delete subnet
///
/// \param[in] key Key
///
/// \return #SDK_RET_OK on success, failure status code on error
///
/// \remark
///  - A valid subnet key should be passed
sdk_ret_t pds_subnet_delete(pds_subnet_key_t *key);

/// \@}

#endif    // __INCLUDE_API_PDS_SUBNET_HPP__
