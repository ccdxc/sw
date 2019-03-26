//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines VCN API
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_PDS_VCN_HPP__
#define __INCLUDE_API_PDS_VCN_HPP__

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/api/include/pds.hpp"

/// \defgroup PDS_VCN VCN API
/// @{

#define PDS_VCN_ID_INVALID 0         ///< Invalid substrate ID
#define PDS_MAX_VCN 1024             ///< Max VCN

/// \brief VCN key
typedef struct pds_vcn_key_s {
    pds_vcn_id_t id;    ///< VCN ID
} __PACK__ pds_vcn_key_t;

/// \brief VCN type
typedef enum pds_vcn_type_e {
    PDS_VCN_TYPE_SUBSTRATE = 0,    ///< Substrate VCN
    PDS_VCN_TYPE_TENANT = 1,       ///< Tenant/Customer VCN
} pds_vcn_type_t;

/// \brief VCN specification
typedef struct pds_vcn_spec_s {
    pds_vcn_key_t key;      ///< Key
    pds_vcn_type_t type;    ///< Type
    ip_prefix_t pfx;        ///< CIDR block
} __PACK__ pds_vcn_spec_t;

/// \brief VCN status
typedef struct pds_vcn_status_s {
    uint16_t hw_id;    ///< Hardware id
} pds_vcn_status_t;

/// \brief VCN statistics
typedef struct pds_vcn_stats_s {
    // TODO
} pds_vcn_stats_t;

/// \brief VCN information
typedef struct pds_vcn_info_s {
    pds_vcn_spec_t spec;        ///< Specification
    pds_vcn_status_t status;    ///< Status
    pds_vcn_stats_t stats;      ///< Statistics
} pds_vcn_info_t;

/// \brief Create VCN
///
/// \param[in] spec Specification
/// \return #SDK_RET_OK on success, failure status code on error
///
/// \remark
///  - A valid VCN ID and prefix should be passed
///  - VCN prefix should not overlap with existing VCN prefixes
///  - A VCN with same ID should not be created again
///  - Any other validation that is expected on the VCN should be done
///    by the caller
sdk_ret_t pds_vcn_create(pds_vcn_spec_t *spec);

/// \brief Read VCN
///
/// \param[in] key Key
/// \param[out] info Information
/// \return #SDK_RET_OK on success, failure status code on error
///
/// \remark
///  - VCN spec containing a valid vcn key should be passed
sdk_ret_t pds_vcn_read(pds_vcn_key_t *key, pds_vcn_info_t *info);

/// \brief Update VCN
///
/// \param[in] spec Specification
/// \return #SDK_RET_OK on success, failure status code on error
///
/// \remark
///  - A valid VCN specification should be passed
sdk_ret_t pds_vcn_update(pds_vcn_spec_t *spec);

/// \brief Delete VCN
///
/// \param[in] key Key
/// \return #SDK_RET_OK on success, failure status code on error
///
/// \remark
///  - A valid VCN key should be passed
sdk_ret_t pds_vcn_delete(pds_vcn_key_t *key);

/// \@}

#endif    // __INCLUDE_API_PDS_VCN_HPP__
