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

#define PDS_SERVICE_TAG_INVALID        0    ///< invalid service tag id

/// \brief  service mapping specification
typedef struct pds_svc_mapping_spec_s {
    pds_obj_key_t key;               ///< service mapping's key (aka. uuid)
    pds_svc_mapping_key_t skey;      ///< secondary key of service mapping
    ip_addr_t vip;                   ///< Virtual IP (VIP) of the service
    uint16_t svc_port;               ///< L4 service port
    ip_addr_t backend_provider_ip;   ///< backend's provider IP
} __PACK__ pds_svc_mapping_spec_t;

/// \brief  service mapping status
typedef struct pds_svc_mapping_status_s {
} __PACK__ pds_svc_mapping_status_t;

/// \brief  service mapping statistics
typedef struct pds_svc_mapping_stats_s {
} __PACK__ pds_svc_mapping_stats_t;

/// \brief  service mapping information
typedef struct pds_svc_mapping_info_t {
    pds_svc_mapping_spec_t spec;        ///< specification
    pds_svc_mapping_status_t status;    ///< status
    pds_svc_mapping_stats_t stats;      ///< statistics
} __PACK__ pds_svc_mapping_info_t;

/// \brief      create service mapping
/// \param[in]  spec service mapping configuration
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return     #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_svc_mapping_create(pds_svc_mapping_spec_t *spec,
                                 pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief      read service mapping
/// \param[in]  key key to service mapping
/// \param[out] info service mapping information
/// \return     #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_svc_mapping_read(pds_obj_key_t *key,
                               pds_svc_mapping_info_t *info);

typedef void (*svc_mapping_read_cb_t)(pds_svc_mapping_info_t *info, void *ctxt);

/// \brief      read all service mapping
/// \param[in] cb     callback for reading service mappings
/// \param[in] ctxt   opaque context to be passed to callbacks
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_svc_mapping_read_all(svc_mapping_read_cb_t cb, void *ctxt);

/// \brief      update service mapping
/// \param[in]  spec service mapping configuration
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return     #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_svc_mapping_update(pds_svc_mapping_spec_t *spec,
                                 pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief      delete service mapping
/// \param[in]  key key to service mapping
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return     #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_svc_mapping_delete(pds_obj_key_t *key,
                                 pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// @}

#endif    // __INCLUDE_API_PDS_SERVICE_HPP__
