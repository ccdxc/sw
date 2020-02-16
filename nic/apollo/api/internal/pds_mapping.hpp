//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file defines mapping APIs for internal module interactions
///
//----------------------------------------------------------------------------

#ifndef __INTERNAL_PDS_MAPPING_HPP__
#define __INTERNAL_PDS_MAPPING_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/mapping.hpp"

namespace api {

/// \brief    read local mapping
/// \param[in] skey    secondary key to local mapping
/// \param[out] info    local mapping information
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_local_mapping_read(pds_mapping_key_t *skey,
                                 pds_local_mapping_info_t *info);

/// \brief    delete local mapping
/// \param[in] skey    secondary key to local mapping
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_local_mapping_delete(pds_mapping_key_t *skey,
                                   pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief    read remote mapping
/// \param[in] skey    secondary key to remote mapping
/// \param[out] info    remote mapping information
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_remote_mapping_read(pds_mapping_key_t *skey,
                                  pds_remote_mapping_info_t *info);

/// \brief    delete remote mapping
/// \param[in] skey    secondary key to remote mapping
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_remote_mapping_delete(pds_mapping_key_t *skey,
                                    pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

}    // namespace api

static inline void
pds_local_spec_to_mapping_spec (pds_mapping_spec_t *spec,
                                pds_local_mapping_spec_t *local_spec)
{
    memset(spec, 0, sizeof(pds_mapping_spec_t));
    spec->key = local_spec->key;
    spec->skey = local_spec->skey;
    spec->subnet = local_spec->subnet;
    spec->fabric_encap = local_spec->fabric_encap;
    memcpy(&spec->overlay_mac, &local_spec->vnic_mac, sizeof(mac_addr_t));
    // local mapping always point to local VTEP (i.e., MyTEP) IP
    spec->nh_type = PDS_NH_TYPE_OVERLAY;
    spec->is_local = true;
    spec->vnic = local_spec->vnic;
    spec->public_ip_valid = local_spec->public_ip_valid;
    spec->public_ip = local_spec->public_ip;
    spec->provider_ip_valid = local_spec->provider_ip_valid;
    spec->provider_ip = local_spec->provider_ip;
    spec->num_tags = local_spec->num_tags;
    for (uint32_t i = 0; i < local_spec->num_tags; i++) {
        spec->tags[i] = local_spec->tags[i];
    }
}

static inline void
pds_remote_spec_to_mapping_spec (pds_mapping_spec_t *spec,
                                 pds_remote_mapping_spec_t *remote_spec)
{
    memset(spec, 0, sizeof(pds_mapping_spec_t));
    spec->key = remote_spec->key;
    spec->skey = remote_spec->skey;
    spec->subnet = remote_spec->subnet;
    spec->fabric_encap = remote_spec->fabric_encap;
    memcpy(&spec->overlay_mac, &remote_spec->vnic_mac, sizeof(mac_addr_t));
    spec->nh_type = remote_spec->nh_type;
    if (spec->nh_type == PDS_NH_TYPE_OVERLAY) {
        spec->tep = remote_spec->tep;
    } else if (spec->nh_type == PDS_NH_TYPE_OVERLAY_ECMP) {
        spec->nh_group = remote_spec->nh_group;
    }
    spec->provider_ip_valid = remote_spec->provider_ip_valid;
    spec->provider_ip = remote_spec->provider_ip;
    spec->is_local = false;
    spec->num_tags = remote_spec->num_tags;
    for (uint32_t i = 0; i < remote_spec->num_tags; i++) {
        spec->tags[i] = remote_spec->tags[i];
    }
}

static inline void
pds_mapping_spec_to_local_spec (pds_local_mapping_spec_t *local_spec,
                                pds_mapping_spec_t *spec)
{
    memset(local_spec, 0, sizeof(pds_local_mapping_spec_t));
    local_spec->key = spec->key;
    local_spec->skey = spec->skey;
    local_spec->subnet = spec->subnet;
    local_spec->fabric_encap = spec->fabric_encap;
    memcpy(&local_spec->vnic_mac, &spec->overlay_mac, sizeof(mac_addr_t));
    local_spec->vnic = spec->vnic;
    local_spec->public_ip_valid = spec->public_ip_valid;
    local_spec->public_ip = spec->public_ip;
    local_spec->provider_ip_valid = spec->provider_ip_valid;
    local_spec->provider_ip = spec->provider_ip;
    local_spec->num_tags = spec->num_tags;
    for (uint32_t i = 0; i < spec->num_tags; i++) {
        local_spec->tags[i] = spec->tags[i];
    }
}

static inline void
pds_mapping_spec_to_remote_spec (pds_remote_mapping_spec_t *remote_spec,
                                 pds_mapping_spec_t *spec)
{
    memset(remote_spec, 0, sizeof(pds_remote_mapping_spec_t));
    remote_spec->key = spec->key;
    remote_spec->skey = spec->skey;
    remote_spec->subnet = spec->subnet;
    remote_spec->fabric_encap = spec->fabric_encap;
    remote_spec->nh_type = spec->nh_type;
    if (spec->nh_type == PDS_NH_TYPE_OVERLAY) {
        remote_spec->tep = spec->tep;
    } else if (spec->nh_type == PDS_NH_TYPE_OVERLAY_ECMP) {
        remote_spec->nh_group = spec->nh_group;
    }
    memcpy(&remote_spec->vnic_mac, &spec->overlay_mac, sizeof(mac_addr_t));
    remote_spec->provider_ip_valid = spec->provider_ip_valid;
    remote_spec->provider_ip = spec->provider_ip;
    remote_spec->num_tags = spec->num_tags;
    for (uint32_t i = 0; i < spec->num_tags; i++) {
        remote_spec->tags[i] = spec->tags[i];
    }
}

#endif    // __INTERNAL_PDS_MAPPING_HPP__
