//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines service mapping APIs
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_PDS_SERVICE_HPP__
#define __INCLUDE_API_PDS_SERVICE_HPP__

#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"

/// \defgroup PDS_SERVICE_MAPPING Mapping API
/// @{

#define PDS_SERVICE_TAG_INVALID    0

/// \brief    service mapping specification
typedef struct pds_svc_mapping_spec_s {
    pds_svc_mapping_key_t key;            ///< service mapping key
    pds_vpc_key_t vpc;                    ///< VPC of the backend
    ip_addr_t     backend_ip;             ///< IP address of the backend
    uint16_t      svc_port;               ///< backend's L4 port
    ip_addr_t     backend_provider_ip;    ///< backend's provider IP
} __PACK__ pds_svc_mapping_spec_t;

/// \brief    service mapping status
typedef struct pds_svc_mapping_status_s {
} __PACK__ pds_svc_mapping_status_t;

/// \brief    service mapping statistics
typedef struct pds_svc_mapping_stats_s {
} __PACK__ pds_svc_mapping_stats_t;

/// \brief    service mapping information
typedef struct pds_svc_mapping_info_t {
    pds_svc_mapping_spec_t spec;         ///< Specification
    pds_svc_mapping_status_t status;    ///< Status
    pds_svc_mapping_stats_t stats;      ///< Statistics
} __PACK__ pds_svc_mapping_info_t;

/// \brief    create service mapping
/// \param[in] spec    service mapping configuration
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_svc_mapping_create(pds_svc_mapping_spec_t *spec);

/// \brief    read service mapping
/// \param[in] key    key to service mapping
/// \param[out] info    service mapping information
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_svc_mapping_read(pds_svc_mapping_key_t *key,
                               pds_svc_mapping_info_t *info);

/// \brief    update service mapping
/// \param[in] spec    service mapping configuration
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_svc_mapping_update(pds_svc_mapping_spec_t *spec);

/// \brief    delete service mapping
/// \param[in] key    key to service mapping
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_svc_mapping_delete(pds_svc_mapping_key_t *key);

/// @}

#endif    // __INCLUDE_API_PDS_SERVICE_HPP__
