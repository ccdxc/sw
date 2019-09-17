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

#define PDS_EPOCH_INVALID             0x0    ///< Invalid epoch
#define PDS_ROUTE_TABLE_ID_INVALID    0x0    ///< Invalid route table id
#define PDS_POLICY_ID_INVALID         0x0    ///< Invalid policy id

// Basic PDS data types
typedef uint32_t                  pds_vpc_id_t;              ///< VPC id
typedef uint32_t                  pds_subnet_id_t;           ///< Subnet id
typedef uint16_t                  pds_vnic_id_t;             ///< VNIC id
typedef uint32_t                  pds_rule_id_t;             ///< Rule index
typedef uint32_t                  pds_rsc_pool_id_t;         ///< Resource pool id
typedef uint32_t                  pds_epoch_t;               ///< Epoch id
typedef uint32_t                  pds_slot_id_t;             ///< MPLS tag value
typedef uint16_t                  pds_vlan_id_t;             ///< VLAN tag value
typedef uint32_t                  pds_vnid_id_t;             ///< VXLAN id
typedef uint32_t                  pds_mpls_tag_t;            ///< MPLS tag value
typedef uint32_t                  pds_mapping_id_t;          ///< Mapping table index
typedef uint32_t                  pds_route_table_id_t;      ///< Route table index
typedef uint32_t                  pds_policy_id_t;           ///< Policy table index
typedef uint16_t                  pds_lif_key_t;             ///< LIF key id
typedef uint32_t                  pds_if_id_t;               ///< Interface id
typedef uint32_t                  pds_mirror_session_id_t;   ///< Mirror session table index
typedef uint32_t                  pds_meter_id_t;            ///< Meter table index
typedef uint32_t                  pds_tag_id_t;              ///< TAG table index
typedef uint32_t                  pds_vpc_peer_id_t;         ///< VPC peer id
typedef uint32_t                  pds_nexthop_id_t;          ///< Nexthop table index
typedef uint32_t                  pds_nexthop_group_id_t;    ///< Nexthop group table index
typedef uint32_t                  pds_tep_id_t;              ///< TEP table index
typedef pds_nexthop_id_t          pds_nexthop_key_t;         ///< Nexthop id
typedef pds_nexthop_group_id_t    pds_nexthop_group_key_t;   ///< Nexthop group id

///< pds_ifindex_t is an internal encoded index used by forwarding and other
///< module to refer to an interface
typedef uint32_t                  pds_ifindex_t;             ///< Interface index

/// \brief Encapsulation type
typedef enum pds_encap_type_e {
    PDS_ENCAP_TYPE_NONE     = 0,    ///< No encap
    PDS_ENCAP_TYPE_DOT1Q    = 1,    ///< dot1q encap
    PDS_ENCAP_TYPE_QINQ     = 2,    ///< QinQ encap
    PDS_ENCAP_TYPE_MPLSoUDP = 3,    ///< MPLSoUDP
    PDS_ENCAP_TYPE_VXLAN    = 4,    ///< VxLAN encap
} pds_encap_type_t;

/// \brief QinQ tag values
typedef struct pds_qinq_tag_s {
    uint16_t c_tag;    ///< Customer VLAN tag
    uint16_t s_tag;    ///< Service VLAN tag
} pds_qinq_tag_t;

/// \brief Encapsulation values
typedef union pds_encap_val_u {
    uint16_t       vlan_tag;    ///< 12 bit .1q tag
    pds_qinq_tag_t qinq_tag;    ///< QinQ tag
    pds_vnid_id_t  vnid;        ///< 24 bit VxLAN vnid
    pds_mpls_tag_t mpls_tag;    ///< 20-bit MPLS tag/slot
    uint32_t       value;       ///< generic value to refer to other values
} pds_encap_val_t;

/// \brief Encapsulation config
typedef struct pds_encap_s {
    pds_encap_type_t type;    ///< Encap type
    pds_encap_val_t  val;     ///< Encap value
} pds_encap_t;

/// \@}

/// \brief    VPC key
typedef struct pds_vpc_key_s {
    pds_vpc_id_t id;    ///< VPC id
} __PACK__ pds_vpc_key_t;

/// \brief    subnet key
/// \remark subnet id is not scoped under a vpc, it is globally unique id
typedef struct pds_subnet_key_s {
    pds_subnet_id_t id;    ///< Subnet id
} __PACK__ pds_subnet_key_t;

/// \brief    TEP key
typedef struct pds_tep_key_s {
    ip_addr_t ip_addr;    ///< TEP IP address
} __PACK__ pds_tep_key_t;

/// \brief    interface key
typedef struct pds_if_key_s {
    pds_if_id_t id;    ///< interface id
} __PACK__ pds_if_key_t;

/// \brief VNIC key
typedef struct pds_vnic_key_s {
    pds_vnic_id_t id;    ///< Unique VNIC id (in the range 0 to 1024)
} __PACK__ pds_vnic_key_t;

/// \brief Meter key
typedef struct pds_meter_key_s {
    pds_meter_id_t id;    ///< Unique meter id
} __PACK__ pds_meter_key_t;

/// \brief Tag key
typedef struct pds_tag_key_s {
    pds_tag_id_t id;    ///< Unique tag id
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
    pds_policy_id_t    id;    ///< Unique id for the policy
} __PACK__ pds_policy_key_t;

/// \brief    mirror session key
typedef struct pds_mirror_session_key_s {
    ///< unique mirror session id in the range [1-8]
    pds_mirror_session_id_t id;
} __PACK__ pds_mirror_session_key_t;

/// \brief    resource pool key
typedef struct pds_rsc_pool_key_s {
    pds_rsc_pool_id_t id;    ///< Resource pool id
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
