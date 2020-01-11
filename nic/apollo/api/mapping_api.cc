//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module implements mapping API
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_msg.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/obj_api.hpp"
#include "nic/apollo/api/mapping.hpp"
#include "nic/apollo/api/include/pds_device.hpp"
#include "nic/apollo/api/include/pds_mapping.hpp"

static sdk_ret_t
pds_mapping_api_handle (pds_batch_ctxt_t bctxt, api_op_t op,
                        pds_mapping_key_t *key, pds_mapping_spec_t *spec)
{
    sdk_ret_t rv;
    api_ctxt_t *api_ctxt;

    if ((rv = pds_obj_api_validate(op, key, spec)) != SDK_RET_OK) {
        return rv;
    }

    api_ctxt = api::api_ctxt_alloc(OBJ_ID_MAPPING, op);
    if (likely(api_ctxt != NULL)) {
        if (op == API_OP_DELETE) {
            api_ctxt->api_params->mapping_key = *key;
        } else {
            api_ctxt->api_params->mapping_spec = *spec;
        }
        return process_api(bctxt, api_ctxt);
    }
    return SDK_RET_OOM;
}

static inline mapping_entry *
pds_mapping_entry_find (pds_mapping_key_t *key)
{
    // mapping does not have any entry database, as the calls are single thread,
    // we can use static entry
    static mapping_entry *mapping;
    pds_mapping_spec_t spec;
    memset(&spec, 0, sizeof(spec));
    spec.key = *key;

    if (mapping == NULL) {
        mapping  = mapping_entry::factory(&spec);
    }
    return mapping;
}

//----------------------------------------------------------------------------
// Mapping API entry point implementation
//----------------------------------------------------------------------------

static inline void
pds_local_spec_to_mapping_spec (pds_mapping_spec_t *spec,
                                pds_local_mapping_spec_t *local_spec)
{
    memset(spec, 0, sizeof(pds_mapping_spec_t));
    spec->key = local_spec->key;
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

//----------------------------------------------------------------------------
// Mapping create routines
//----------------------------------------------------------------------------

sdk_ret_t
pds_local_mapping_create (_In_ pds_local_mapping_spec_t *local_spec,
                          _In_ pds_batch_ctxt_t bctxt)
{
    pds_mapping_spec_t spec;

    pds_local_spec_to_mapping_spec(&spec, local_spec);
    return pds_mapping_api_handle(bctxt, API_OP_CREATE, NULL, &spec);
}

sdk_ret_t
pds_remote_mapping_create (_In_ pds_remote_mapping_spec_t *remote_spec,
                           _In_ pds_batch_ctxt_t bctxt)
{
    pds_mapping_spec_t spec;

    if ((remote_spec->nh_type != PDS_NH_TYPE_OVERLAY) &&
        (remote_spec->nh_type != PDS_NH_TYPE_OVERLAY_ECMP)) {
        return SDK_RET_INVALID_ARG;
    }
    pds_remote_spec_to_mapping_spec(&spec, remote_spec);
    return pds_mapping_api_handle(bctxt, API_OP_CREATE, NULL, &spec);
}

//----------------------------------------------------------------------------
// mapping read routines
//----------------------------------------------------------------------------

sdk_ret_t
pds_local_mapping_read (pds_mapping_key_t *key,
                        pds_local_mapping_info_t *local_info)
{
    pds_mapping_info_t info;
    mapping_entry *entry = NULL;
    sdk_ret_t rv = SDK_RET_OK;

    if (key == NULL || local_info == NULL)
        return SDK_RET_INVALID_ARG;

    if ((entry = pds_mapping_entry_find(key)) == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    info.spec.key = *key;
    entry->set_local(true);
    rv = entry->read(key, &info);
    if (rv == SDK_RET_OK) {
        pds_mapping_spec_to_local_spec(&local_info->spec, &info.spec);
    }
    return rv;
}

sdk_ret_t
pds_remote_mapping_read (pds_mapping_key_t *key,
                         pds_remote_mapping_info_t *remote_info)
{
    pds_mapping_info_t info;
    mapping_entry *entry = NULL;
    sdk_ret_t rv = SDK_RET_OK;

    if (key == NULL || remote_info == NULL) {
        return SDK_RET_INVALID_ARG;
    }

    if ((entry = pds_mapping_entry_find(key)) == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    info.spec.key = *key;
    entry->set_local(false);
    rv = entry->read(key, &info);
    if (rv == SDK_RET_OK) {
        pds_mapping_spec_to_remote_spec(&remote_info->spec, &info.spec);
    }
    return rv;
}

//----------------------------------------------------------------------------
// Mapping update routines
//----------------------------------------------------------------------------

sdk_ret_t
pds_local_mapping_update (_In_ pds_local_mapping_spec_t *local_spec,
                          _In_ pds_batch_ctxt_t bctxt)
{
    pds_mapping_spec_t spec;

    pds_local_spec_to_mapping_spec(&spec, local_spec);
    return pds_mapping_api_handle(bctxt, API_OP_UPDATE, NULL, &spec);
}

sdk_ret_t
pds_remote_mapping_update (_In_ pds_remote_mapping_spec_t *remote_spec,
                           _In_ pds_batch_ctxt_t bctxt)
{
    pds_mapping_spec_t spec;

    pds_remote_spec_to_mapping_spec(&spec, remote_spec);
    return pds_mapping_api_handle(bctxt, API_OP_UPDATE, NULL, &spec);
}

//----------------------------------------------------------------------------
// Mapping delete routines
//----------------------------------------------------------------------------

sdk_ret_t
pds_local_mapping_delete (_In_ pds_mapping_key_t *key,
                          _In_ pds_batch_ctxt_t bctxt)
{
    return pds_mapping_api_handle(bctxt, API_OP_DELETE, key, NULL);
}

sdk_ret_t
pds_remote_mapping_delete (_In_ pds_mapping_key_t *key,
                           _In_ pds_batch_ctxt_t bctxt)
{
    return pds_mapping_api_handle(bctxt, API_OP_DELETE, key, NULL);
}
