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
#include "nic/apollo/api/include/pds_nexthop.hpp"

/// \defgroup PDS_MAPPING Mapping API
/// @{

///< maximum of 5 tags per local/remote IP mapping
#define PDS_MAX_TAGS_PER_MAPPING        5

/// \brief    local mapping specification
typedef struct pds_local_mapping_spec_s {
    pds_mapping_key_t key;                    ///< mapping key
    pds_obj_key_t vnic;                       ///< vnic for given IP
    pds_obj_key_t subnet;                     ///< subnet this IP is part of
    pds_encap_t fabric_encap;                 ///< fabric encap for this mapping
    mac_addr_t vnic_mac;                      ///< vnic MAC
    bool public_ip_valid;                     ///< true if public IP is valid
    ip_addr_t public_ip;                      ///< public IP address
    bool provider_ip_valid;                   ///< true if provider IP is valid
    ip_addr_t provider_ip;                    ///< provider IP address
    /// number of tag/labels/security groups for the mapping
    uint32_t num_tags;
    /// tags/labels/security groups for the mapping
    uint32_t  tags[PDS_MAX_TAGS_PER_MAPPING];
} __PACK__ pds_local_mapping_spec_t;

/// \brief    remote mapping specification
typedef struct pds_remote_mapping_spec_s {
    pds_mapping_key_t key;       ///< mapping key
    pds_obj_key_t subnet;     ///< subnet this IP is part of
    pds_encap_t fabric_encap;    ///< fabric encap for this mapping
    pds_nh_type_t nh_type;       ///< type of the nexthop for this mapping
    union {
        ///< remote TEP where this mapping is located
        pds_obj_key_t tep;
        ///< overlay nexthop group for this mapping
        pds_obj_key_t nh_group;
    };
    mac_addr_t vnic_mac;         ///< remote vnic's MAC
    /// number of tag/labels/security groups for the mapping
    uint32_t num_tags;
    /// tags/labels/security groups for the mapping
    uint32_t  tags[PDS_MAX_TAGS_PER_MAPPING];
    bool provider_ip_valid;      ///< true if provider IP is valid
    ip_addr_t provider_ip;       ///< provider IP address
} __PACK__ pds_remote_mapping_spec_t;

/// \brief    mapping status
typedef struct pds_mapping_status_s {
    uint16_t subnet_hw_id; ///< subnet hw id
    uint16_t vnic_hw_id;   ///< vnic hw id
} __PACK__ pds_mapping_status_t;

/// \brief    mapping statistics
typedef struct pds_mapping_stats_s {
} __PACK__ pds_mapping_stats_t;

/// \brief    local mapping information
typedef struct pds_local_mapping_info_t {
    pds_local_mapping_spec_t spec;    ///< specification
    pds_mapping_status_t status;      ///< status
    pds_mapping_stats_t stats;        ///< statistics
} __PACK__ pds_local_mapping_info_t;

/// \brief    remote mapping information
typedef struct pds_remote_mapping_info_t {
    pds_remote_mapping_spec_t spec;    ///< specification
    pds_mapping_status_t status;       ///< status
    pds_mapping_stats_t stats;         ///< statistics
} __PACK__ pds_remote_mapping_info_t;

/// \brief    create local mapping
/// \param[in] spec    local mapping configuration
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_local_mapping_create(pds_local_mapping_spec_t *spec,
                                   pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief    read local mapping
/// \param[in] key    key to local mapping
/// \param[out] info    local mapping information
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_local_mapping_read(pds_mapping_key_t *key,
                                 pds_local_mapping_info_t *info);

/// \brief    update local mapping
/// \param[in] spec    local mapping configuration
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_local_mapping_update(pds_local_mapping_spec_t *spec,
                                   pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief    delete local mapping
/// \param[in] key    key to local mapping
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_local_mapping_delete(pds_mapping_key_t *key,
                                   pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief    create remote mapping
/// \param[in] spec    remote mapping configuration
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_remote_mapping_create(pds_remote_mapping_spec_t *spec,
                                    pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief    read remote mapping
/// \param[in] key    key to remote mapping
/// \param[out] info    remote mapping information
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_remote_mapping_read(pds_mapping_key_t *key,
                                  pds_remote_mapping_info_t *info);

/// \brief    update remote mapping
/// \param[in] spec    remote mapping configuration
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_remote_mapping_update(pds_remote_mapping_spec_t *spec,
                                    pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief    delete remote mapping
/// \param[in] key    key to remote mapping
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_remote_mapping_delete(pds_mapping_key_t *key,
                                    pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// @}

#endif    // __INCLUDE_API_PDS_MAPPING_HPP__
