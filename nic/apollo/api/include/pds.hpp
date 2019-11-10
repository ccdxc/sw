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
#include "nic/sdk/lib/ht/ht.hpp"

/// \defgroup PDS_BASE Base enums and macros
/// @{

#define PDS_EPOCH_INVALID             0x0    ///< invalid epoch
#define PDS_BATCH_CTXT_INVALID        0x0    ///< invalid batch context
#define PDS_ROUTE_TABLE_ID_INVALID    0x0    ///< invalid route table id
#define PDS_POLICY_ID_INVALID         0x0    ///< invalid policy id
#define PDS_PORT_INVALID              0xFF   ///< invalid port#

// Basic PDS data types
typedef uint64_t    pds_batch_ctxt_t;          ///< opaque batch context
typedef uint32_t    pds_vpc_id_t;              ///< vpc id
typedef uint32_t    pds_subnet_id_t;           ///< subnet id
typedef uint16_t    pds_vnic_id_t;             ///< vnic id
typedef uint32_t    pds_rule_id_t;             ///< rule identifier
typedef uint32_t    pds_rsc_pool_id_t;         ///< resource pool id
typedef uint32_t    pds_epoch_t;               ///< epoch id
typedef uint32_t    pds_slot_id_t;             ///< MPLS tag value
typedef uint16_t    pds_vlan_id_t;             ///< VLAN tag value
typedef uint32_t    pds_vnid_id_t;             ///< VxLAN id
typedef uint32_t    pds_mpls_tag_t;            ///< MPLS tag value
typedef uint32_t    pds_mapping_id_t;          ///< mapping table index
typedef uint32_t    pds_route_table_id_t;      ///< route table index
typedef uint32_t    pds_policy_id_t;           ///< policy table index
typedef uint16_t    pds_lif_key_t;             ///< lif key id
typedef uint32_t    pds_if_id_t;               ///< interface id
typedef uint32_t    pds_mirror_session_id_t;   ///< mirror session table index
typedef uint32_t    pds_meter_id_t;            ///< meter table index
typedef uint32_t    pds_tag_id_t;              ///< tag table index
typedef uint32_t    pds_vpc_peer_id_t;         ///< vpc peer id
typedef uint32_t    pds_nexthop_id_t;          ///< nexthop table index
typedef uint32_t    pds_nexthop_group_id_t;    ///< nexthop group table index
typedef uint32_t    pds_tep_id_t;              ///< TEP table index
typedef uint32_t    pds_ifindex_t;             ///< interface index

///< pds_ifindex_t is an internal encoded index used by forwarding and other
///< module to refer to an interface

/// \brief encapsulation type
typedef enum pds_encap_type_e {
    PDS_ENCAP_TYPE_NONE     = 0,    ///< No encap
    PDS_ENCAP_TYPE_DOT1Q    = 1,    ///< dot1q encap
    PDS_ENCAP_TYPE_QINQ     = 2,    ///< QinQ encap
    PDS_ENCAP_TYPE_MPLSoUDP = 3,    ///< MPLSoUDP
    PDS_ENCAP_TYPE_VXLAN    = 4,    ///< VxLAN encap
} pds_encap_type_t;

/// \brief QinQ tag values
typedef struct pds_qinq_tag_s {
    uint16_t c_tag;    ///< customer VLAN tag
    uint16_t s_tag;    ///< service VLAN tag
} pds_qinq_tag_t;

/// \brief encapsulation values
typedef union pds_encap_val_u {
    uint16_t       vlan_tag;    ///< 12 bit .1q tag
    pds_qinq_tag_t qinq_tag;    ///< QinQ tag
    pds_vnid_id_t  vnid;        ///< 24 bit VxLAN vnid
    pds_mpls_tag_t mpls_tag;    ///< 20-bit MPLS tag/slot
    uint32_t       value;       ///< generic value to refer to other values
} pds_encap_val_t;

/// \brief encapsulation config
typedef struct pds_encap_s {
    pds_encap_type_t type;    ///< encap type
    pds_encap_val_t  val;     ///< encap value
} pds_encap_t;

static inline char *
pds_encap2str (const pds_encap_t *encap)
{
    static thread_local char       encap_str[4][20];
    static thread_local uint8_t    encap_str_next = 0;
    char                           *buf;

    buf = encap_str[encap_str_next++ & 0x3];
    switch (encap->type) {
    case PDS_ENCAP_TYPE_DOT1Q:
        snprintf(buf, 20, "Dot1q/%u", encap->val.value);
        break;
    case PDS_ENCAP_TYPE_QINQ:
        snprintf(buf, 20, "QinQ/%u", encap->val.value);
        break;
    case PDS_ENCAP_TYPE_MPLSoUDP:
        snprintf(buf, 20, "MPLSoUDP/%u", encap->val.value);
        break;
    case PDS_ENCAP_TYPE_VXLAN:
        snprintf(buf, 20, "VxLAN/%u", encap->val.value);
        break;
    default:
        snprintf(buf, 20, "-");
        break;
    }
    return buf;
}

/// \brief NAT type
typedef enum pds_nat_type_e {
    NAT_TYPE_NONE         = 0,
    NAT_TYPE_STATIC       = 1,
    NAT_TYPE_DYNAMIC_ADDR = 2,
    NAT_TYPE_DYNAMIC_NAPT = 3,
} pds_nat_type_t;

/// \brief NAT action
typedef struct pds_nat_action_s {
    pds_nat_type_t src_nat;
    pds_nat_type_t dst_nat;
} pds_nat_action_t;

/// \brief    VPC key
typedef struct pds_vpc_key_s {
    pds_vpc_id_t id;    ///< VPC id
} __PACK__ pds_vpc_key_t;

/// \brief    subnet key
/// \remark subnet id is not scoped under a vpc, it is globally unique id
typedef struct pds_subnet_key_s {
    pds_subnet_id_t id;    ///< subnet id
} __PACK__ pds_subnet_key_t;

/// \brief    TEP key
typedef struct pds_tep_key_s {
    pds_tep_id_t id;    ///< TEP ID
} __PACK__ pds_tep_key_t;

/// \brief    interface key
typedef struct pds_if_key_s {
    pds_if_id_t id;    ///< interface id
} __PACK__ pds_if_key_t;

/// \brief VNIC key
typedef struct pds_vnic_key_s {
    pds_vnic_id_t id;    ///< unique VNIC id (in the range 0 to 1024)
} __PACK__ pds_vnic_key_t;

/// \brief Meter key
typedef struct pds_meter_key_s {
    pds_meter_id_t id;    ///< unique meter id
} __PACK__ pds_meter_key_t;

/// \brief Tag key
typedef struct pds_tag_key_s {
    pds_tag_id_t id;    ///< unique tag id
} __PACK__ pds_tag_key_t;

/// \brief    mapping type
typedef enum pds_mapping_type_e {
    PDS_MAPPING_TYPE_NONE = 0,
    PDS_MAPPING_TYPE_L2   = 1,
    PDS_MAPPING_TYPE_L3   = 2,
} pds_mapping_type_t;

/// \brief    mapping key
typedef struct pds_mapping_key_s {
    pds_mapping_type_t type;
    ///< L3 key
    struct {
        pds_vpc_key_t vpc;    ///< VPC this IP belongs to
        ip_addr_t ip_addr;    ///< IP address of the mapping
    };
    ///< L2 key
    struct {
        pds_subnet_key_t subnet;    ///< subnet of the mapping
        mac_addr_t mac_addr;        ///< MAC address of the mapping
    };
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
    pds_policy_id_t    id;    ///< unique id for the policy
} __PACK__ pds_policy_key_t;

/// \brief    mirror session key
typedef struct pds_mirror_session_key_s {
    ///< unique mirror session id in the range [1-8]
    pds_mirror_session_id_t id;
} __PACK__ pds_mirror_session_key_t;

/// \brief nexthop key
typedef struct pds_nexthop_key_s {
    pds_nexthop_id_t  id;    ///< unique id of nexthop
} __PACK__ pds_nexthop_key_t;

/// \brief nexthop group key
typedef struct pds_nexthop_group_key_s {
    pds_nexthop_group_id_t  id;    ///< unique id of nexthop group
} __PACK__ pds_nexthop_group_key_t;

/// \brief    resource pool key
typedef struct pds_rsc_pool_key_s {
    pds_rsc_pool_id_t id;    ///< resource pool id
} __PACK__ pds_rsc_pool_key_t;

/// \brief    service mapping key
typedef struct pds_svc_mapping_key_s {
    pds_vpc_key_t vpc;    ///< VPC this service is in
    ip_addr_t vip;        ///< Virtual IP (VIP) of the service
    uint16_t svc_port;    ///< L4 service port

    // operator== is required to compare keys in case of hash collision
    /// compare operator
    bool operator==(const struct pds_svc_mapping_key_s &p) const {
        return (vpc.id == p.vpc.id) && (svc_port == p.svc_port) &&
            (!memcmp(&vip, &p.vip, sizeof(ip_addr_t)));
    }
} __PACK__ pds_svc_mapping_key_t;

/// \brief    service mapping hash function
class pds_svc_mapping_hash_fn {
public:
    /// function call operator
    std::size_t operator()(const pds_svc_mapping_key_t &key) const {
        return hash_algo::fnv_hash((void *)&key, sizeof(key));
    }
};

/// \brief    vpc peering key
typedef struct pds_vpc_peer_key_s {
    pds_vpc_peer_id_t id;    ///< VPC peer id
} __PACK__ pds_vpc_peer_key_t;

/// @}
#endif    // __INCLUDE_API_PDS_HPP__
