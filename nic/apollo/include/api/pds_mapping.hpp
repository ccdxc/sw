//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines mapping API
///
///----------------------------------------------------------------------------

#ifndef __INCLUDE_API_PDS_MAPPING_HPP__
#define __INCLUDE_API_PDS_MAPPING_HPP__

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/include/api/pds.hpp"
#include "nic/apollo/include/api/pds_tep.hpp"
#include "nic/apollo/include/api/pds_vcn.hpp"
#include "nic/apollo/include/api/pds_subnet.hpp"
#include "nic/apollo/include/api/pds_vnic.hpp"

/// \defgroup PDS_MAPPING Mapping API
/// @{

/// \brief Mapping key
typedef struct pds_mapping_key_s {
    pds_vcn_key_t vcn;    ///< VCN this IP belongs to
    ip_addr_t ip_addr;    ///< IP address of the mapping
} __PACK__ pds_mapping_key_t;

/// \brief Mapping specification
typedef struct pds_mapping_spec_s {
    pds_mapping_key_t key;      ///< Mapping key
    pds_subnet_key_t subnet;    ///< Subnet this IP is part of
    pds_slot_id_t slot;         ///< Virtual slot (Encap: MPLS Tag)
    pds_tep_key_t tep;          ///< TEP address for this mapping
                                ///< 1. Switchport IP for local vnic
                                ///< 2. Remote TEP for remote vnic
    mac_addr_t overlay_mac;     ///< MAC for this IP

    ///< Information specific to local IP mappings
    struct {
        pds_vnic_key_t vnic;     ///< VNIC for local IP
        bool public_ip_valid;    ///< TRUE if public IP is valid
        ip_addr_t public_ip;     ///< Public IP address
    };
} pds_mapping_spec_t;

/// \brief Mapping status
typedef struct pds_mapping_status_s {
    // TODO
} pds_mapping_status_t;

/// \brief Mapping statistics
typedef struct pds_mapping_stats_s {
    // TODO
} pds_mapping_stats_t;

/// \brief Mapping information
typedef struct pds_mapping_info_s {
    pds_mapping_spec_t spec;        ///< Specification
    pds_mapping_status_t status;    ///< Status
    pds_mapping_stats_t stats;      ///< Statistics
} pds_mapping_info_t;

/// \brief Create IP mapping
///
/// \param[in] spec Specification
///
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_mapping_create(pds_mapping_spec_t *spec);

/// \brief Read IP mapping
///
/// \param[in] key Key
/// \param[out] info Information
///
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_mapping_read(pds_mapping_key_t *key, pds_mapping_info_t *info);

/// \brief Update IP mapping
///
/// \param[in] spec Specification
///
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_mapping_update(pds_mapping_spec_t *spec);

/// \brief Delete IP mapping
///
/// \param[in] key Key
///
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_mappnig_delete(pds_mapping_key_t *key);

/// @}

#endif    // __INCLUDE_API_PDS_MAPPING_HPP__
