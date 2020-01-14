//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines VPC API
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_PDS_VPC_HPP__
#define __INCLUDE_API_PDS_VPC_HPP__

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/include/pds_route.hpp"

/// \defgroup PDS_VPC VPC API
/// @{

//#define PDS_VPC_ID_INVALID 0     ///< invalid VPC id

///< invalid VPC key
#define PDS_VPC_ID_INVALID    k_pds_obj_key_invalid
#define PDS_MAX_VPC        64    ///< max VPC

/// \brief VPC type
typedef enum pds_vpc_type_e {
    PDS_VPC_TYPE_NONE     = 0,
    PDS_VPC_TYPE_UNDERLAY = 1,    ///< underlay VPC
    PDS_VPC_TYPE_TENANT   = 2,    ///< tenant/customer VPC
} pds_vpc_type_t;

/// \brief VPC specification
typedef struct pds_vpc_spec_s {
    pds_obj_key_t         key;               ///< key
    pds_vpc_type_t        type;              ///< type
    ipv4_prefix_t         v4_prefix;         ///< IPv4 CIDR block
    ip_prefix_t           v6_prefix;         ///< IPv6 CIDR block
    ///< traffic routed in this VPC will carry SMAC as vr_mac, if
    ///< there are no subnets configured (or else the vr_mac configured
    ///< in the subnet will stamped as SMAC in the routed packet),
    ///< additionally, only packets coming with DMAC as this vr_mac are routed
    mac_addr_t            vr_mac;            ///< vnic's overlay MAC address
    pds_encap_t           fabric_encap;      ///< fabric encap for this VPC
    ///< subnets of this VPC configured without route table
    ///< will inherit corresponding VPC's route table(s), if any
    pds_route_table_key_t v4_route_table;    ///< IPv4 route table id
    pds_route_table_key_t v6_route_table;    ///< IPv6 route table id
    ip_prefix_t           nat46_prefix;      ///< IPv6 prefix/IP to be used to
                                             ///< (S)NAT outbound traffic from
                                             ///< this VPC that is destined to
                                             ///< a IPv6-only service/network
    /// type of service to be used in the outer header in encapped pkts when
    /// vpc's vnid is used in the packet
    /// NOTE: this tos value is not inherited by subnets inside this vpc (they
    ///       need to be configured explicitly with either same tos value as
    ///       their vpc or a different value
    uint8_t tos;
} __PACK__ pds_vpc_spec_t;

/// \brief VPC status
typedef struct pds_vpc_status_s {
    uint16_t hw_id;    ///< hardware id
} __PACK__ pds_vpc_status_t;

/// \brief VPC statistics
typedef struct pds_vpc_stats_s {
    // TODO
} __PACK__ pds_vpc_stats_t;

/// \brief VPC information
typedef struct pds_vpc_info_s {
    pds_vpc_spec_t   spec;      ///< specification
    pds_vpc_status_t status;    ///< status
    pds_vpc_stats_t  stats;     ///< statistics
} __PACK__ pds_vpc_info_t;

/// \brief     create VPC
/// \param[in] spec specification
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid VPC ID and prefix should be passed
///            VPC prefix should not overlap with existing VPC prefixes
///            A VPC with same ID should not be created again
///            Any other validation that is expected on the VPC should be done
///            by the caller
sdk_ret_t pds_vpc_create(pds_vpc_spec_t *spec,
                         pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief      read VPC
/// \param[in]  key  key
/// \param[out] info information
/// \return     #SDK_RET_OK on success, failure status code on error
/// \remark     VPC spec containing a valid vpc key should be passed
sdk_ret_t pds_vpc_read(pds_obj_key_t *key, pds_vpc_info_t *info);

/// \brief     update VPC
/// \param[in] spec specification
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid VPC specification should be passed
sdk_ret_t pds_vpc_update(pds_vpc_spec_t *spec,
                         pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief     delete VPC
/// \param[in] key key
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid VPC key should be passed
sdk_ret_t pds_vpc_delete(pds_obj_key_t *key,
                         pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief VPC peering specification
typedef struct pds_vpc_peer_spec_s {
    pds_obj_key_t         key;    ///< key
    pds_obj_key_t         vpc1;   ///< vpc1 key
    pds_obj_key_t         vpc2;   ///< vpc2 VPC
} __PACK__ pds_vpc_peer_spec_t;

/// \brief VPC peering status
typedef struct pds_vpc_peer_status_s {
    uint16_t hw_id1;    ///< h/w id for (vpc1, vpc2) entry
    uint16_t hw_id2;    ///< h/w id for (vpc2, vpc1) entry
} __PACK__ pds_vpc_peer_status_t;

/// \brief VPC peerings statistics, if any
typedef struct pds_vpc_peer_stats_s {
} __PACK__ pds_vpc_peer_stats_t;

/// \brief VPC peering information
typedef struct pds_vpc_peer_info_s {
    pds_vpc_peer_spec_t spec;        ///< specification
    pds_vpc_peer_status_t status;    ///< operational status
    pds_vpc_peer_stats_t stats;      ///< statistics
} __PACK__ pds_vpc_peer_info_t;

/// \brief     create VPC peering
/// \param[in] spec specification
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return    #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_vpc_peer_create(pds_vpc_peer_spec_t *spec,
                              pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief      read VPC peering related information
/// \param[in]  key  key
/// \param[out] info information
/// \return     #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_vpc_peer_read(pds_obj_key_t *key, pds_vpc_peer_info_t *info);

/// \brief     update VPC peering
/// \param[in] spec specification
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid VPC specification should be passed
sdk_ret_t pds_vpc_peer_update(pds_vpc_peer_spec_t *spec,
                              pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief     delete VPC peering
/// \param[in] key key
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return    #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_vpc_peer_delete(pds_obj_key_t *key,
                              pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// @}

#endif    // __INCLUDE_API_PDS_VPC_HPP__
