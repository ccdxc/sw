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

/// \defgroup PDS_VPC VPC API
/// @{

#define PDS_VPC_ID_INVALID 0         ///< Invalid substrate ID
#define PDS_MAX_VPC 1024             ///< Max VPC

/// \brief VPC key
typedef struct pds_vpc_key_s {
    pds_vpc_id_t id;    ///< VPC ID
} __PACK__ pds_vpc_key_t;

/// \brief VPC type
typedef enum pds_vpc_type_e {
    PDS_VPC_TYPE_NONE      = 0,
    PDS_VPC_TYPE_SUBSTRATE = 1,    ///< Substrate VPC
    PDS_VPC_TYPE_TENANT    = 2,    ///< Tenant/Customer VPC
} pds_vpc_type_t;

/// \brief VPC specification
typedef struct pds_vpc_spec_s {
    pds_vpc_key_t key;       ///< Key
    pds_vpc_type_t type;     ///< Type
    ipv4_prefix_t v4_pfx;    ///< IPv4 CIDR block
    ip_prefix_t v6_pfx;      ///< IPv6 CIDR block
} __PACK__ pds_vpc_spec_t;

/// \brief VPC status
typedef struct pds_vpc_status_s {
    uint16_t hw_id;    ///< Hardware id
} pds_vpc_status_t;

/// \brief VPC statistics
typedef struct pds_vpc_stats_s {
    // TODO
} pds_vpc_stats_t;

/// \brief VPC information
typedef struct pds_vpc_info_s {
    pds_vpc_spec_t spec;        ///< Specification
    pds_vpc_status_t status;    ///< Status
    pds_vpc_stats_t stats;      ///< Statistics
} pds_vpc_info_t;

/// \brief Create VPC
///
/// \param[in] spec Specification
/// \return #SDK_RET_OK on success, failure status code on error
///
/// \remark
///  - A valid VPC ID and prefix should be passed
///  - VPC prefix should not overlap with existing VPC prefixes
///  - A VPC with same ID should not be created again
///  - Any other validation that is expected on the VPC should be done
///    by the caller
sdk_ret_t pds_vpc_create(pds_vpc_spec_t *spec);

/// \brief Read VPC
///
/// \param[in] key Key
/// \param[out] info Information
/// \return #SDK_RET_OK on success, failure status code on error
///
/// \remark
///  - VPC spec containing a valid vpc key should be passed
sdk_ret_t pds_vpc_read(pds_vpc_key_t *key, pds_vpc_info_t *info);

/// \brief Update VPC
///
/// \param[in] spec Specification
/// \return #SDK_RET_OK on success, failure status code on error
///
/// \remark
///  - A valid VPC specification should be passed
sdk_ret_t pds_vpc_update(pds_vpc_spec_t *spec);

/// \brief Delete VPC
///
/// \param[in] key Key
/// \return #SDK_RET_OK on success, failure status code on error
///
/// \remark
///  - A valid VPC key should be passed
sdk_ret_t pds_vpc_delete(pds_vpc_key_t *key);

/// \@}

#endif    // __INCLUDE_API_PDS_VPC_HPP__
