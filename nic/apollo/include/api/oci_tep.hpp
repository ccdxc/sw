//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
///----------------------------------------------------------------------------
///
/// \file
/// This module defines TEP API
///
///----------------------------------------------------------------------------

#ifndef __INCLUDE_API_OCI_TEP_HPP__
#define __INCLUDE_API_OCI_TEP_HPP__

#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/include/api/oci.hpp"

/// \defgroup OCI_TEP Tunnel End Point API
/// @{

#define OCI_MAX_TEP 1024

/// \brief Encapsulation type
typedef enum oci_encap_type_e {
    OCI_ENCAP_TYPE_NONE = 0,        ///< No encap
    OCI_ENCAP_TYPE_GW_ENCAP = 1,    ///< MPLSoUDP with single mpls label
    OCI_ENCAP_TYPE_VNIC = 2,        ///< MPLSoUDP with two mpls labels
} oci_encap_type_t;

/// \brief TEP key
typedef struct oci_tep_key_s {
    ipv4_addr_t ip_addr;    ///< TEP IP address
} __PACK__ oci_tep_key_t;

/// \brief TEP specification
typedef struct oci_tep_s {
    oci_tep_key_t key;        ///< Key
    oci_encap_type_t type;    ///< Encapsulation Type
} __PACK__ oci_tep_spec_t;

/// \brief TEP status
typedef struct oci_tep_status_s {
    uint16_t nh_id;                ///< Next hop id for this TEP
    uint16_t hw_id;                ///< Hardware id
    uint8_t dmac[ETH_ADDR_LEN];    ///< Outer destination MAC
} __PACK__ oci_tep_status_t;

/// \brief TEP statistics
typedef struct oci_tep_stats_s {
    // TODO: No Stats for TEP
} __PACK__ oci_tep_stats_t;

/// \brief TEP information
typedef struct oci_tep_info_s {
    oci_tep_spec_t spec;        ///< Specification
    oci_tep_status_t status;    ///< Status
    oci_tep_stats_t stats;      ///< Statistics
} __PACK__ oci_tep_info_t;

/// \brief Create TEP
///
/// \param[in] spec Specification
///
/// \return #SDK_RET_OK on success, failure status code on error
///
/// \remark
///  - A TEP with same key (i.e., IP address) should not be created again
///  - Any other validation that is expected on the TEP should be done
///    by the caller
sdk_ret_t oci_tep_create(oci_tep_spec_t *spec);

/// \brief Read TEP
///
/// \param[in] key Key
/// \param[out] info Information
///
/// \return #SDK_RET_OK on success, failure status code on error
///
/// \remark
///  - TEP spec containing a valid tep key should be passed
sdk_ret_t oci_tep_read(oci_tep_key_t *key, oci_tep_info_t *info);

/// \brief Update TEP
///
/// \param[in] spec Specification
///
/// \return #SDK_RET_OK on success, failure status code on error
///
/// \remark
///  - A valid TEP specification should be passed
sdk_ret_t oci_tep_update(oci_tep_spec_t *spec);

/// \brief Delete TEP
///
/// \param[in] key Key
///
/// \return #SDK_RET_OK on success, failure status code on error
///
/// \remark
///  - A valid TEP key should be passed
sdk_ret_t oci_tep_delete(oci_tep_key_t *key);

/// \@}

#endif    // __INCLUDE_API_OCI_TEP_HPP__
