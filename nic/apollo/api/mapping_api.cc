//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module implements Mapping API
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/obj_api.hpp"
#include "nic/apollo/api/mapping.hpp"
#include "nic/apollo/api/impl/mapping_impl.hpp"
#include "nic/apollo/api/include/pds_device.hpp"
#include "nic/apollo/api/include/pds_mapping.hpp"

static sdk_ret_t
pds_mapping_api_handle (api::api_op_t op, pds_mapping_key_t *key,
                        pds_mapping_spec_t *spec)
{
    sdk_ret_t rv;
    api_ctxt_t api_ctxt;

    if ((rv = pds_obj_api_validate(op, key, spec)) != SDK_RET_OK)
        return rv;

    api_ctxt.api_params = api::api_params_alloc(api::OBJ_ID_MAPPING, op);
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = op;
        api_ctxt.obj_id = api::OBJ_ID_MAPPING;
        if (op == api::API_OP_DELETE)
            api_ctxt.api_params->mapping_key = *key;
        else
            api_ctxt.api_params->mapping_spec = *spec;
        rv = api::g_api_engine.process_api(&api_ctxt);
        return rv;
    }
    return sdk::SDK_RET_OOM;
}

static inline mapping_entry *
pds_mapping_entry_find (pds_mapping_key_t *key)
{
    pds_mapping_spec_t spec = {0};
    spec.key = *key;
    spec.is_local = true;

    // Mapping does not have any entry database
    // As the call are single thread, we can use static entry
    static mapping_entry *mapping;

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
    spec->key.vcn.id = local_spec->key.vcn.id;
    spec->key.ip_addr = local_spec->key.ip_addr;
    spec->subnet.id = local_spec->subnet.id;
    spec->fabric_encap = local_spec->fabric_encap;
    memcpy(&spec->overlay_mac, &local_spec->vnic_mac, sizeof(mac_addr_t));
    spec->public_ip_valid = local_spec->public_ip_valid;
    spec->public_ip = local_spec->public_ip;
    spec->vnic.id = local_spec->vnic.id;
    spec->is_local = true;
}

static inline void
pds_remote_spec_to_mapping_spec (pds_mapping_spec_t *spec,
                                 pds_remote_mapping_spec_t *remote_spec)
{
    memset(spec, 0, sizeof(pds_mapping_spec_t));
    spec->key = remote_spec->key;
    spec->subnet = remote_spec->subnet;
    spec->fabric_encap = remote_spec->fabric_encap;
    spec->tep = remote_spec->tep;
    memcpy(&spec->overlay_mac, &remote_spec->vnic_mac, sizeof(mac_addr_t));
    spec->is_local = false;
}

static inline void
pds_mapping_spec_to_local_spec (pds_local_mapping_spec_t *local_spec,
                                pds_mapping_spec_t *spec)
{
    memset(local_spec, 0, sizeof(pds_local_mapping_spec_t));
    local_spec->key.vcn.id = spec->key.vcn.id;
    local_spec->key.ip_addr = spec->key.ip_addr;
    local_spec->subnet.id = spec->subnet.id;
    local_spec->fabric_encap = spec->fabric_encap;
    memcpy(&local_spec->vnic_mac, &spec->overlay_mac, sizeof(mac_addr_t));
    local_spec->vnic.id = spec->vnic.id;
    spec->public_ip_valid = local_spec->public_ip_valid;
    local_spec->public_ip = spec->public_ip;
}

static inline void
pds_mapping_spec_to_remote_spec (pds_remote_mapping_spec_t *remote_spec,
                                 pds_mapping_spec_t *spec)
{
    memset(remote_spec, 0, sizeof(pds_remote_mapping_spec_t));
    remote_spec->key = spec->key;
    remote_spec->subnet = spec->subnet;
    remote_spec->fabric_encap = spec->fabric_encap;
    remote_spec->tep = spec->tep;
    memcpy(&remote_spec->vnic_mac, &spec->overlay_mac, sizeof(mac_addr_t));
}

//----------------------------------------------------------------------------
// Mapping create routines
//----------------------------------------------------------------------------

sdk_ret_t
pds_local_mapping_create (pds_local_mapping_spec_t *local_spec)
{
    pds_mapping_spec_t spec;

    pds_local_spec_to_mapping_spec(&spec, local_spec);
    return (pds_mapping_api_handle(api::API_OP_CREATE, NULL, &spec));
}

sdk_ret_t
pds_remote_mapping_create (pds_remote_mapping_spec_t *remote_spec)
{
    pds_mapping_spec_t spec;

    pds_remote_spec_to_mapping_spec(&spec, remote_spec);
    return (pds_mapping_api_handle(api::API_OP_CREATE, NULL, &spec));
}

//----------------------------------------------------------------------------
// Mapping read routines
//----------------------------------------------------------------------------

sdk_ret_t
pds_local_mapping_read (pds_mapping_key_t *key,
                        pds_local_mapping_info_t *local_info)
{
    pds_mapping_info_t info;
    mapping_entry *entry = NULL;
    api::impl::mapping_impl *impl;
    sdk_ret_t rv = SDK_RET_OK;

    if (key == NULL || local_info == NULL)
        return SDK_RET_INVALID_ARG;

    if ((entry = pds_mapping_entry_find(key)) == NULL)
        return SDK_RET_ENTRY_NOT_FOUND;

    info.spec.key = *key;
    info.spec.vnic.id = local_info->spec.vnic.id;
    impl = dynamic_cast<api::impl::mapping_impl*>(entry->impl());
    rv = impl->read_hw(key, &info);
    pds_mapping_spec_to_local_spec(&local_info->spec, &info.spec);
    return rv;
}

sdk_ret_t
pds_remote_mapping_read (pds_mapping_key_t *key,
                         pds_remote_mapping_info_t *remote_info)
{
    pds_mapping_info_t info;
    mapping_entry *entry = NULL;
    api::impl::mapping_impl *impl;
    sdk_ret_t rv = SDK_RET_OK;

    if (key == NULL || remote_info == NULL)
        return SDK_RET_INVALID_ARG;

    if ((entry = pds_mapping_entry_find(key)) == NULL)
        return SDK_RET_ENTRY_NOT_FOUND;

    info.spec.key = *key;
    impl = dynamic_cast<api::impl::mapping_impl*>(entry->impl());
    rv = impl->read_hw(key, &info);
    pds_mapping_spec_to_remote_spec(&remote_info->spec, &info.spec);
    return rv;
}

//----------------------------------------------------------------------------
// Mapping update routines
//----------------------------------------------------------------------------

sdk_ret_t
pds_local_mapping_update (pds_local_mapping_spec_t *local_spec)
{
    pds_mapping_spec_t spec;

    pds_local_spec_to_mapping_spec(&spec, local_spec);
    return (pds_mapping_api_handle(api::API_OP_UPDATE, NULL, &spec));
}

sdk_ret_t
pds_remote_mapping_update (pds_remote_mapping_spec_t *remote_spec)
{
    pds_mapping_spec_t spec;

    pds_remote_spec_to_mapping_spec(&spec, remote_spec);
    return (pds_mapping_api_handle(api::API_OP_UPDATE, NULL, &spec));
}

//----------------------------------------------------------------------------
// Mapping delete routines
//----------------------------------------------------------------------------

sdk_ret_t
pds_local_mapping_delete (pds_mapping_key_t *key)
{
    return (pds_mapping_api_handle(api::API_OP_DELETE, key, NULL));
}

sdk_ret_t
pds_remote_mapping_delete (pds_mapping_key_t *key)
{
    return (pds_mapping_api_handle(api::API_OP_DELETE, key, NULL));
}
