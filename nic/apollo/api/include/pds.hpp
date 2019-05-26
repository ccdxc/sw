//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// Base enums and macros for PDS APIs
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_PDS_HPP__
#define __INCLUDE_API_PDS_HPP__

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include "nic/sdk/include/sdk/ip.hpp"

/// \defgroup PDS_BASE Base enums and macros
/// @{

#define PDS_EPOCH_INVALID             0x0    ///< Invalid epoch
#define PDS_ROUTE_TABLE_ID_INVALID    0x0    ///< Invalid route table ID
#define PDS_POLICY_ID_INVALID         0x0    ///< Invalid policy ID

// Basic PDS data types
typedef uint32_t pds_vpc_id_t;
typedef uint32_t pds_subnet_id_t;
typedef uint16_t pds_vnic_id_t;
typedef uint32_t pds_rule_id_t;
typedef uint32_t pds_rsc_pool_id_t;
typedef uint32_t pds_epoch_t;
typedef uint32_t pds_slot_id_t;
typedef uint32_t pds_vnid_id_t;
typedef uint32_t pds_mpls_tag_t;
typedef uint32_t pds_mapping_id_t;
typedef uint32_t pds_route_table_id_t;
typedef uint32_t pds_policy_id_t;
typedef uint16_t pds_lif_key_t;
typedef uint32_t pds_ifindex_t;
typedef uint32_t pds_mirror_session_id_t;
typedef uint32_t pds_meter_id_t;
typedef uint32_t pds_tag_id_t;

/// \brief Encapsulation type
typedef enum pds_encap_type_e {
    PDS_ENCAP_TYPE_NONE     = 0,    ///< No encap
    PDS_ENCAP_TYPE_DOT1Q    = 1,    ///< dot1q encap
    PDS_ENCAP_TYPE_QINQ     = 2,    ///< QinQ encap
    PDS_ENCAP_TYPE_MPLSoUDP = 3,    ///< MPLSoUDP
    PDS_ENCAP_TYPE_VXLAN    = 4,    ///< VxLAN encap
} pds_encap_type_t;

typedef struct pds_qinq_tag_s {
    uint16_t c_tag;    ///< Customer VLAN tag
    uint16_t s_tag;    ///< Service VLAN tag
} pds_qinq_tag_t;

typedef union pds_encap_val_u {
    uint16_t       vlan_tag;    ///< 12 bit .1q tag
    pds_qinq_tag_t qinq_tag;    ///< QinQ tag
    pds_vnid_id_t  vnid;        ///< 24 bit VxLAN vnid
    pds_mpls_tag_t mpls_tag;    ///< 20-bit MPLS tag/slot
    uint32_t       value;       ///< generic value to refer to other values
} pds_encap_val_t;

typedef struct pds_encap_s {
    pds_encap_type_t type;
    pds_encap_val_t  val;
} pds_encap_t;

/// \@}

static inline char *
pdsencap2str (pds_encap_t encap)
{
    static thread_local char buf_pool[4][32];
    static thread_local int buf_next = 0;
    char *buf;

    buf = buf_pool[buf_next++ % 4];
    switch (encap.type) {
    case PDS_ENCAP_TYPE_DOT1Q:
        snprintf(buf, 32, "Dot1q:%u", encap.val.vlan_tag);
        break;
    case PDS_ENCAP_TYPE_QINQ:
        snprintf(buf, 32, "QinQ:%u/%u",
                 encap.val.qinq_tag.c_tag, encap.val.qinq_tag.s_tag);
        break;
    case PDS_ENCAP_TYPE_MPLSoUDP:
        snprintf(buf, 32, "MPLSoUDP:%u", encap.val.mpls_tag);
        break;
    case PDS_ENCAP_TYPE_VXLAN:
        snprintf(buf, 32, "VXLAN:%u", encap.val.vnid);
        break;
    default:
        break;
    }
    return buf;
}

/// \brief    VPC key
typedef struct pds_vpc_key_s {
    pds_vpc_id_t id;    ///< VPC ID
} __PACK__ pds_vpc_key_t;

/// \brief    subnet key
/// \remark subnet id is not scoped under a vpc, it is globally unique id
typedef struct pds_subnet_key_s {
    pds_subnet_id_t id;    ///< Subnet ID
} __PACK__ pds_subnet_key_t;

/// \brief    TEP key
typedef struct pds_tep_key_s {
    ipv4_addr_t ip_addr;    ///< TEP IP address
} __PACK__ pds_tep_key_t;

/// \brief VNIC key
typedef struct pds_vnic_key_s {
    pds_vnic_id_t id;    ///< Unique VNIC ID (in the range 0 to 1024)
} __PACK__ pds_vnic_key_t;

/// \brief Meter key
typedef struct pds_meter_key_s {
    pds_meter_id_t id;    ///< Unique meter ID
} __PACK__ pds_meter_key_t;

/// \brief Tag key
typedef struct pds_tag_key_s {
    pds_tag_id_t id;    ///< Unique tag ID
} __PACK__ pds_tag_key_t;

/// \brief    mapping key
typedef struct pds_mapping_key_s {
    pds_vpc_key_t vpc;    ///< VPC this IP belongs to
    ip_addr_t ip_addr;    ///< IP address of the mapping
} __PACK__ pds_mapping_key_t;

/// \brief    route table key
/// \remark
///  - Route table id is not scoped under vpc, it is unique on
//     the device (across VPCs and IPv4/IPv6 route tables)
typedef struct pds_route_table_key_s {
    pds_route_table_id_t    id;    ///< route table id
} __PACK__ pds_route_table_key_t;

/// \brief    policy key
typedef struct pds_policy_key_s {
    pds_policy_id_t    id;    ///< Unique ID for the policy
} __PACK__ pds_policy_key_t;

/// \brief    mirror session key
typedef struct pds_mirror_session_key_s {
    ///< unique mirror session id in the range [1-8]
    pds_mirror_session_id_t id;
} __PACK__ pds_mirror_session_key_t;

/// \brief    resource pool key
typedef struct pds_rsc_pool_key_s {
    pds_rsc_pool_id_t id;    ///< Resource pool ID
} __PACK__ pds_rsc_pool_key_t;

/// \brief    service mapping key
typedef struct pds_svc_mapping_key_s {
    pds_vpc_key_t vpc;    ///< VPC this service is in
    ip_addr_t vip;        ///< Virtual IP (VIP) of the service
    uint16_t svc_port;    ///< L4 service port
} __PACK__ pds_svc_mapping_key_t;

/// \brief    vpc peering key
typedef struct pds_vpc_peer_key_s {
    pds_vpc_key_t vpc1;    ///< vpc1 key
    pds_vpc_key_t vpc2;    ///< vpc2 VPC
} __PACK__ pds_vpc_peer_key_t;

#endif    // __INCLUDE_API_PDS_HPP__
