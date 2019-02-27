//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines mapping API
///
///----------------------------------------------------------------------------

#ifndef __INCLUDE_API_OCI_MAPPING_HPP__
#define __INCLUDE_API_OCI_MAPPING_HPP__

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/include/api/oci.hpp"
#include "nic/apollo/include/api/oci_tep.hpp"
#include "nic/apollo/include/api/oci_vcn.hpp"
#include "nic/apollo/include/api/oci_subnet.hpp"
#include "nic/apollo/include/api/oci_vnic.hpp"

/// \defgroup OCI_MAPPING Mapping API
/// @{

/// \brief Mapping key
typedef struct oci_mapping_key_s {
    oci_vcn_key_t vcn;    ///< VCN this IP belongs to
    ip_addr_t ip_addr;    ///< IP address of the mapping
} __PACK__ oci_mapping_key_t;

/// \brief Mapping specification
typedef struct oci_mapping_spec_s {
    oci_mapping_key_t key;      ///< Mapping key
    oci_subnet_key_t subnet;    ///< Subnet this IP is part of
    oci_slot_id_t slot;         ///< Virtual slot (Encap: MPLS Tag)
    oci_tep_key_t tep;          ///< TEP address for this mapping
                                ///< 1. Switchport IP for local vnic
                                ///< 2. Remote TEP for remote vnic
    mac_addr_t overlay_mac;     ///< MAC for this IP

    ///< Information specific to local IP mappings
    struct {
        oci_vnic_key_t vnic;     ///< VNIC for local IP
        bool public_ip_valid;    ///< TRUE if public IP is valid
        ip_addr_t public_ip;     ///< Public IP address
    };
} oci_mapping_spec_t;

/// \brief Mapping status
typedef struct oci_mapping_status_s {
    // TODO
} oci_mapping_status_t;

/// \brief Mapping statistics
typedef struct oci_mapping_stats_s {
    // TODO
} oci_mapping_stats_t;

/// \brief Mapping information
typedef struct oci_mapping_info_s {
    oci_mapping_spec_t spec;        ///< Specification
    oci_mapping_status_t status;    ///< Status
    oci_mapping_stats_t stats;      ///< Statistics
} oci_mapping_info_t;

/// \brief Create IP mapping
///
/// \param[in] spec Specification
///
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t oci_mapping_create(oci_mapping_spec_t *spec);

/// \brief Read IP mapping
///
/// \param[in] key Key
/// \param[out] info Information
///
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t oci_mapping_read(oci_mapping_key_t *key, oci_mapping_info_t *info);

/// \brief Update IP mapping
///
/// \param[in] spec Specification
///
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t oci_mapping_update(oci_mapping_spec_t *spec);

/// \brief Delete IP mapping
///
/// \param[in] key Key
///
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t oci_mappnig_delete(oci_mapping_key_t *key);

/// @}

#endif    // __INCLUDE_API_OCI_MAPPING_HPP__
