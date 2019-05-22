//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines mapping APIs
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_PDS_MAPPING_HPP__
#define __INCLUDE_API_PDS_MAPPING_HPP__

#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"

/// \defgroup PDS_MAPPING Mapping API
/// @{

/// \brief    local mapping specification
typedef struct pds_local_mapping_spec_s {
    pds_mapping_key_t key;       ///< Mapping key
    pds_vnic_key_t vnic;         ///< VNIC for given IP
    pds_subnet_key_t subnet;     ///< Subnet this IP is part of
    pds_encap_t fabric_encap;    ///< fabric encap for this mapping
    mac_addr_t vnic_mac;         ///< VNIC MAC
    bool public_ip_valid;        ///< TRUE if public IP is valid
    ip_addr_t public_ip;         ///< Public IP address
    bool provider_ip_valid;      ///< TRUE if provider IP is valid
    ip_addr_t provider_ip;       ///< Provider IP address
    uint32_t  svc_tag;           ///< service tag, if any
} __PACK__ pds_local_mapping_spec_t;

/// \brief    remote mapping specification
typedef struct pds_remote_mapping_spec_s {
    pds_mapping_key_t key;       ///< Mapping key
    pds_subnet_key_t subnet;     ///< Subnet this IP is part of
    pds_encap_t fabric_encap;    ///< fabric encap for this mapping
    pds_tep_key_t tep;           ///< Remote TEP address for this mapping
    mac_addr_t vnic_mac;         ///< VNIC MAC
} __PACK__ pds_remote_mapping_spec_t;

/// \brief    mapping status
typedef struct pds_mapping_status_s {
} __PACK__ pds_mapping_status_t;

/// \brief    mapping statistics
typedef struct pds_mapping_stats_s {
} __PACK__ pds_mapping_stats_t;

/// \brief    local mapping information
typedef struct pds_local_mapping_info_t {
    pds_local_mapping_spec_t spec;    ///< Specification
    pds_mapping_status_t status;      ///< Status
    pds_mapping_stats_t stats;        ///< Statistics
} __PACK__ pds_local_mapping_info_t;

/// \brief    remote mapping information
typedef struct pds_remote_mapping_info_t {
    pds_remote_mapping_spec_t spec;    ///< Specification
    pds_mapping_status_t status;       ///< Status
    pds_mapping_stats_t stats;         ///< Statistics
} __PACK__ pds_remote_mapping_info_t;

/// \brief    create local mapping
/// \param[in] spec    local mapping configuration
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_local_mapping_create(pds_local_mapping_spec_t *spec);

/// \brief    read local mapping
/// \param[in] key    key to local mapping
/// \param[out] info    local mapping information
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_local_mapping_read(pds_mapping_key_t *key,
                                 pds_local_mapping_info_t *info);

/// \brief    update local mapping
/// \param[in] spec    local mapping configuration
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_local_mapping_update(pds_local_mapping_spec_t *spec);

/// \brief    delete local mapping
/// \param[in] key    key to local mapping
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_local_mapping_delete(pds_mapping_key_t *key);

/// \brief    create remote mapping
/// \param[in] spec    remote mapping configuration
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_remote_mapping_create(pds_remote_mapping_spec_t *spec);

/// \brief    read remote mapping
/// \param[in] key    key to remote mapping
/// \param[out] info    remote mapping information
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_remote_mapping_read(pds_mapping_key_t *key,
                                  pds_remote_mapping_info_t *info);

/// \brief    update remote mapping
/// \param[in] spec    remote mapping configuration
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_remote_mapping_update(pds_remote_mapping_spec_t *spec);

/// \brief    delete remote mapping
/// \param[in] key    key to remote mapping
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_remote_mapping_delete(pds_mapping_key_t *key);

/// @}

#endif    // __INCLUDE_API_PDS_MAPPING_HPP__
