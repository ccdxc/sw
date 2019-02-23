//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines VCN API
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_VCN_HPP__
#define __INCLUDE_API_VCN_HPP__

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/include/api/oci.hpp"

/// \defgroup OCI_VCN Virtual Cloud Network
/// @{

#define OCI_VCN_ID_INVALID 0xFFFF    ///< Invalid substrate ID
#define OCI_VCN_ID_SUBSTRATE 0       ///< VCN ID reserved for substrate
#define OCI_MAX_VCN 1024

/// \brief VCN key
typedef struct oci_vcn_key_s {
    oci_vcn_id_t id;    /// VCN ID
} __PACK__ oci_vcn_key_t;

/// \brief VCN type
typedef enum oci_vcn_type_e {
    OCI_VCN_TYPE_SUBSTRATE = 0,    ///< Substrate VCN
    OCI_VCN_TYPE_TENANT = 1,       ///< Tenant/Customer VCN
} oci_vcn_type_t;

/// \brief VCN specification
typedef struct oci_vcn_spec_s {
    oci_vcn_key_t key;      ///< Key
    oci_vcn_type_t type;    ///< Type
    ip_prefix_t pfx;        ///< CIDR block
} __PACK__ oci_vcn_spec_t;

/// \brief VCN status
typedef struct oci_vcn_status_s {
    uint16_t hw_id;    ///< Hardware id
} oci_vcn_status_t;

/// \brief VCN statistics
typedef struct oci_vcn_stats_s {
    // TODO
} oci_vcn_stats_t;

/// \brief VCN info
typedef struct oci_vcn_info_s {
    oci_vcn_spec_t spec;
    oci_vcn_status_t status;
    oci_vcn_stats_t stats;
} oci_vcn_info_t;

/// \brief Create VCN
///
/// \param[in] spec VCN specification
///
/// \return #SDK_RET_OK on success, failure status code on error
///
/// \remark
///  - A valid VCN ID and prefix should be passed
///  - VCN prefix should not overlap with existing VCN prefixes
///  - A VCN with same ID should not be created again
///  - Any other validation that is expected on the VCN should be done
///    by the caller
sdk_ret_t oci_vcn_create(oci_vcn_spec_t *spec);

/// \brief Read VCN
///
/// \param[in] key VCN key
/// \param[out] spec VCN specification
/// \param[out] status VCN status
/// \param[out] stats VCN statistics
///
/// \return #SDK_RET_OK on success, failure status code on error
///
/// \remark
///  - VCN spec containing a valid vcn key should be passed
sdk_ret_t oci_vcn_read(oci_vcn_key_t *key, oci_vcn_info_t *info);

/// \brief Update VCN
///
/// \param[in] spec VCN specification
///
/// \return #SDK_RET_OK on success, failure status code on error
///
/// \remark
///  - A valid VCN specification should be passed
sdk_ret_t oci_vcn_update(oci_vcn_spec_t *spec);

/// \brief Delete VCN
///
/// \param[in] key VCN key
///
/// \return #SDK_RET_OK on success, failure status code on error
///
/// \remark
///  - A valid VCN key should be passed
sdk_ret_t oci_vcn_delete(oci_vcn_key_t *key);

/// \@}

#endif    // __INCLUDE_API_VCN_HPP__
