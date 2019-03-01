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

/// \defgroup PDS_BASE Base enums and macros
/// @{

#define PDS_EPOCH_INVALID             0x0    ///< Invalid epoch
#define PDS_ROUTE_TABLE_ID_INVALID    0x0    ///< Invalid route table ID
#define PDS_POLICY_ID_INVALID         0x0    ///< Invalid policy ID

// Basic PDS data types
typedef uint32_t  pds_vcn_id_t;
typedef uint32_t  pds_subnet_id_t;
typedef uint16_t  pds_vnic_id_t;
typedef uint32_t  pds_rule_id_t;
typedef uint32_t  pds_rsc_pool_id_t;
typedef uint32_t  pds_epoch_t;
typedef uint32_t  pds_slot_id_t;
typedef uint32_t  pds_vnid_id_t;
typedef uint32_t  pds_mpls_tag_t;
typedef uint32_t  pds_mapping_id_t;
typedef uint32_t  pds_route_table_id_t;
typedef uint32_t  pds_policy_id_t;

/// \brief Encapsulation type
typedef enum pds_encap_type_e {
    PDS_ENCAP_TYPE_NONE     = 0,    ///< No encap
    PDS_ENCAP_TYPE_DOT1Q    = 1,    ///< dot1q encap
    PDS_ENCAP_TYPE_QINQ     = 2,    ///< QinQ encap
    PDS_ENCAP_TYPE_MPLSoUDP = 3,    ///< MPLSoUDP
    PDS_ENCAP_TYPE_VXLAN    = 4,    ///< VxLAN encap
} pds_encap_type_t;

typedef union pds_encap_val_u {
    uint16_t          vlan_tag;    ///< 12 bit .1q tag
    pds_vnid_id_t     vnid;        ///< 24 bit VxLAN vnid
    pds_mpls_tag_t    mpls_tag;    ///< 20-bit MPLS tag/slot
    uint32_t          value;       ///< generic value to refer to other values
} pds_encap_val_t;

typedef struct pds_encap_s {
    pds_encap_type_t    type;
    pds_encap_val_t     val;
} pds_encap_t;

/// \@}

#endif    // __INCLUDE_API_PDS_HPP__
