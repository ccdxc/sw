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
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/apollo/api/include/pds_policy.hpp"
#include "nic/apollo/api/include/pds_route.hpp"

/// \defgroup PDS_SUBNET Subnet API
/// @{

#define PDS_MAX_SUBNET           64    ///< max subnets
#define PDS_MAX_SUBNET_POLICY    5     ///< max #of security policies per subnet

/// \brief Subnet specification
typedef struct pds_subnet_spec_s {
    pds_subnet_key_t key;                    ///< key
    pds_vpc_key_t vpc;                       ///< VPC key
    ipv4_prefix_t v4_prefix;                 ///< IPv4 CIDR block
    ip_prefix_t v6_prefix;                   ///< IPv6 CIDR block
    ipv4_addr_t v4_vr_ip;                    ///< IPv4 virtual router IP
    ip_addr_t v6_vr_ip;                      ///< IPv6 virtual router IP
    mac_addr_t vr_mac;                       ///< virtual router mac
    pds_route_table_key_t v4_route_table;    ///< IPv4 Route table id
    pds_route_table_key_t v6_route_table;    ///< IPv6 Route table id
    ///< ingress IPv4 policy table(s)
    uint8_t num_ing_v4_policy;
    pds_policy_key_t ing_v4_policy[PDS_MAX_SUBNET_POLICY];
    ///< ingress IPv6 policy table(s)
    uint8_t num_ing_v6_policy;
    pds_policy_key_t ing_v6_policy[PDS_MAX_SUBNET_POLICY];
    ///< egress IPv4 policy table(s)
    uint8_t num_egr_v4_policy;
    pds_policy_key_t egr_v4_policy[PDS_MAX_SUBNET_POLICY];
    ///< egress IPv6 policy table(s)
    uint8_t num_egr_v6_policy;
    pds_policy_key_t egr_v6_policy[PDS_MAX_SUBNET_POLICY];
    pds_encap_t fabric_encap;                ///< fabric encap for this subnet
    /// when operating in PDS_DEV_OPER_MODE_HOST mode with multiple host
    /// PFs/VFs present, subnet can be attached to PF/VF
    pds_ifindex_t host_ifindex;
    pds_dhcp_policy_key_t dhcp_policy;       ///< DHCP policy, if any
    uint8_t tos;                             ///< type of service to be used
                                             ///< in the outer header in
                                             ///< encapped pkts
} __PACK__ pds_subnet_spec_t;

/// \brief Subnet status
typedef struct pds_subnet_status_s {
    uint16_t hw_id;                 ///< hardware id
    mem_addr_t policy_base_addr;    ///< policy base address
} __PACK__ pds_subnet_status_t;

/// \brief Subnet statistics
typedef struct pds_subnet_stats_s {
    // TODO
} __PACK__ pds_subnet_stats_t;

/// \brief Subnet information
typedef struct pds_subnet_info_s {
    pds_subnet_spec_t spec;        ///< specification
    pds_subnet_status_t status;    ///< status
    pds_subnet_stats_t stats;      ///< statistics
} __PACK__ pds_subnet_info_t;

/// \brief Create subnet
/// \param[in] spec Specification
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return #SDK_RET_OK on success, failure status code on error
/// \remark
///  - A valid VPC id should be used
///  - Subnet prefix passed should be valid as per VPC prefix
///  - Subnet prefix should not overlap with any other subnet
///  - Subnet with same id should not be created again
///  - Any other validation that is expected on the subnet should be done
///    by the caller
sdk_ret_t pds_subnet_create(pds_subnet_spec_t *spec,
                            pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief Read subnet
/// \param[in] key Key
/// \param[out] info Information
/// \return #SDK_RET_OK on success, failure status code on error
/// \remark
///  - Subnet spec containing a valid subnet key should be passed
sdk_ret_t pds_subnet_read(pds_subnet_key_t *key, pds_subnet_info_t *info);

/// \brief Update subnet
/// \param[in] spec Specification
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return #SDK_RET_OK on success, failure status code on error
/// \remark
///  - A valid subnet spec should be passed
sdk_ret_t pds_subnet_update(pds_subnet_spec_t *spec,
                            pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief Delete subnet
/// \param[in] key Key
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return #SDK_RET_OK on success, failure status code on error
/// \remark
///  - A valid subnet key should be passed
sdk_ret_t pds_subnet_delete(pds_subnet_key_t *key,
                            pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// @}

#endif    // __INCLUDE_API_PDS_SUBNET_HPP__
