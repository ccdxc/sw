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
typedef uint32_t  pds_mapping_id_t;
typedef uint32_t  pds_route_table_id_t;
typedef uint32_t  pds_policy_id_t;

/// \@}

#endif    // __INCLUDE_API_PDS_HPP__
