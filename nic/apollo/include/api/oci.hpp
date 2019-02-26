//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// Base enums and macros for OCI APIs
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_OCI_HPP__
#define __INCLUDE_API_OCI_HPP__

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

/// \defgroup OCI_BASE Base enums and macros
/// @{

#define OCI_EPOCH_INVALID             0x0    ///< Invalid epoch
#define OCI_ROUTE_TABLE_ID_INVALID    0x0    ///< Invalid route table ID
#define OCI_POLICY_ID_INVALID         0x0    ///< Invalid policy ID

// Basic OCI data types
typedef uint32_t  oci_vcn_id_t;
typedef uint32_t  oci_subnet_id_t;
typedef uint16_t  oci_vnic_id_t;
typedef uint32_t  oci_rule_id_t;
typedef uint32_t  oci_rsc_pool_id_t;
typedef uint32_t  oci_epoch_t;
typedef uint32_t  oci_slot_id_t;
typedef uint32_t  oci_mapping_id_t;
typedef uint32_t  oci_route_table_id_t;
typedef uint32_t  oci_policy_id_t;

/// \@}

#endif    // __INCLUDE_API_OCI_HPP__
