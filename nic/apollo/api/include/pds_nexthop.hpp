//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines nexthop and nexthop group APIs
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_PDS_NEXTHOP_HPP__
#define __INCLUDE_API_PDS_NEXTHOP_HPP__

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/apollo/api/include/pds.hpp"

/// \defgroup PDS_NEXTHOP nexthop & nexthop group APIs
/// @{

#define PDS_MAX_NEXTHOP                4095    ///< Maximum nexthops
#define PDS_MAX_NEXTHOP_GROUP          1024    ///< Maximum nexthop groups

/// \brief nexthop type
typedef enum pds_nh_type_e {
    PDS_NH_TYPE_NONE      = 0,
    PDS_NH_TYPE_BLACKHOLE = 1,    ///< blackhole/drop nexthop
    PDS_NH_TYPE_TEP       = 2,    ///< any of the possible types of TEP
    PDS_NH_TYPE_IP        = 3,    ///< native IP route
    PDS_NH_TYPE_PEER_VPC  = 4,    ///< VPC id of the peer VPC
} pds_nh_type_t;

/// \brief nexthop specification
typedef struct pds_nexthop_spec_s {
    pds_nexthop_key_t     key;     ///< key
    pds_nh_type_t         type;    ///< nexthop type
    union {
        // info specific to PDS_NH_TYPE_IP
        struct {
            pds_vpc_key_t vpc;     ///< nexthop's (egress VPC)
            ip_addr_t     ip;      ///< nexthop IP address
            uint16_t      vlan;    ///< egress vlan encap (for tagged packets)
            mac_addr_t    mac;     ///< (optional) MAC address if known at
                                   ///< config time
        };
    };
} __PACK__ pds_nexthop_spec_t;

/// \brief nexthop status
typedef struct pds_nexthop_status_s {
    uint16_t hw_id;    ///< hardware id
} __PACK__ pds_nexthop_status_t;

/// \brief nexthop statistics
typedef struct pds_nexthop_stats_s {
    // TODO
} __PACK__ pds_nexthop_stats_t;

/// \brief nexthop information
typedef struct pds_nexthop_info_s {
    pds_nexthop_spec_t   spec;      ///< specification
    pds_nexthop_status_t status;    ///< status
    pds_nexthop_stats_t  stats;     ///< statistics
} __PACK__ pds_nexthop_info_t;

/// \brief     create nexthop
/// \param[in] spec nexthop specification
/// \return    #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_nexthop_create(pds_nexthop_spec_t *spec);

/// \brief      read a given nexthop
/// \param[in]  key  key of the nexthop
/// \param[out] info nexthop information
/// \return     #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_nexthop_read(pds_nexthop_key_t *key, pds_nexthop_info_t *info);

/// \brief     update nexthop
/// \param[in] spec nexthop specification
/// \return    #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_nexthop_update(pds_nexthop_spec_t *spec);

/// \brief     delete a given nexthop
/// \param[in] key key of the nexthop
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid nexthop key should be passed
sdk_ret_t pds_nexthop_delete(pds_nexthop_key_t *key);

/// \brief nexthop group type
typedef enum pds_nexthop_group_type_e {
    PDS_NHGROUP_TYPE_NONE          = 0,
    PDS_NHGROUP_TYPE_OVERLAY_ECMP  = 1,    ///< overlay ECMP nexthop group
    PDS_NHGROUP_TYPE_UNDERLAY_ECMP = 2,    ///< underlay ECMP nexthop group
} pds_nexthop_group_type_t;

/// \brief nexthop group specification
typedef struct pds_nexthop_group_spec_s {
    pds_nexthop_group_key_t key;      ///< key
    pds_nexthop_group_type_t type;    ///< nexthop group type
    uint16_t num_nexthops;            ///< number of nexthops in this group
    pds_nexthop_key_t nexthops[0];    ///< nexthop list of this group
} __PACK__ pds_nexthop_group_spec_t;

/// \brief nexthop group status
typedef struct pds_nexthop_group_status_s {
    uint16_t hw_id;    ///< hardware id
} __PACK__ pds_nexthop_group_status_t;

/// \brief nexthop group statistics
typedef struct pds_nexthop_group_stats_s {
    // TODO
} __PACK__ pds_nexthop_group_stats_t;

/// \brief nexthop group information
typedef struct pds_nexthop_group_info_s {
    pds_nexthop_group_spec_t   spec;      ///< specification
    pds_nexthop_group_status_t status;    ///< status
    pds_nexthop_group_stats_t  stats;     ///< statistics
} __PACK__ pds_nexthop_group_info_t;

/// @}

#endif    // __INCLUDE_API_PDS_NEXTHOP_HPP__
