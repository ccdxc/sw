//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
///----------------------------------------------------------------------------
///
/// \file
/// This module defines TEP API
///
///----------------------------------------------------------------------------

#ifndef __INCLUDE_API_PDS_TEP_HPP__
#define __INCLUDE_API_PDS_TEP_HPP__

#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/include/api/pds.hpp"

/// \defgroup PDS_TEP Tunnel End Point API
/// @{

#define PDS_MAX_TEP 1024

/// \brief Encapsulation type
typedef enum pds_tep_encap_type_e {
    PDS_TEP_ENCAP_TYPE_NONE     = 0,    ///< No encap
    ///< MPLSoUDP encap types
    PDS_TEP_ENCAP_TYPE_GW_ENCAP = 1,    ///< MPLSoUDP with single mpls label
    PDS_TEP_ENCAP_TYPE_VNIC     = 2,    ///< MPLSoUDP with two mpls labels
    ///< VxLAN encap
    PDS_TEP_ENCAP_TYPE_VXLAN    = 3,    ///< VxLAN encap
} pds_tep_encap_type_t;

/// \brief TEP key
typedef struct pds_tep_key_s {
    ipv4_addr_t ip_addr;    ///< TEP IP address
} __PACK__ pds_tep_key_t;

/// \brief TEP specification
typedef struct pds_tep_spec_s {
    pds_tep_key_t key;                  ///< Key
    pds_tep_encap_type_t encap_type;    ///< Encapsulation Type
} __PACK__ pds_tep_spec_t;

/// \brief TEP status
typedef struct pds_tep_status_s {
    uint16_t nh_id;                ///< Next hop id for this TEP
    uint16_t hw_id;                ///< Hardware id
    uint8_t dmac[ETH_ADDR_LEN];    ///< Outer destination MAC
} __PACK__ pds_tep_status_t;

/// \brief TEP statistics
typedef struct pds_tep_stats_s {
    // TODO: No Stats for TEP
} __PACK__ pds_tep_stats_t;

/// \brief TEP information
typedef struct pds_tep_info_s {
    pds_tep_spec_t spec;        ///< Specification
    pds_tep_status_t status;    ///< Status
    pds_tep_stats_t stats;      ///< Statistics
} __PACK__ pds_tep_info_t;

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
sdk_ret_t pds_tep_create(pds_tep_spec_t *spec);

/// \brief Read TEP
///
/// \param[in] key Key
/// \param[out] info Information
///
/// \return #SDK_RET_OK on success, failure status code on error
///
/// \remark
///  - TEP spec containing a valid tep key should be passed
sdk_ret_t pds_tep_read(pds_tep_key_t *key, pds_tep_info_t *info);

/// \brief Update TEP
///
/// \param[in] spec Specification
///
/// \return #SDK_RET_OK on success, failure status code on error
///
/// \remark
///  - A valid TEP specification should be passed
sdk_ret_t pds_tep_update(pds_tep_spec_t *spec);

/// \brief Delete TEP
///
/// \param[in] key Key
///
/// \return #SDK_RET_OK on success, failure status code on error
///
/// \remark
///  - A valid TEP key should be passed
sdk_ret_t pds_tep_delete(pds_tep_key_t *key);

/// \@}

#endif    // __INCLUDE_API_PDS_TEP_HPP__
