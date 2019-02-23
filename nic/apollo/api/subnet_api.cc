//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file implements subnet CRUD API
///
//----------------------------------------------------------------------------

#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/obj_api.hpp"
#include "nic/apollo/api/oci_state.hpp"
#include "nic/apollo/api/subnet.hpp"
#include "nic/apollo/api/subnet_state.hpp"

static sdk::sdk_ret_t
oci_subnet_api_handle (api::api_op_t op, oci_subnet_key_t *key,
                       oci_subnet_spec_t *spec)
{
    sdk::sdk_ret_t rv;
    api_ctxt_t api_ctxt;

    if ((rv = oci_obj_api_validate(op, key, spec)) != sdk::SDK_RET_OK)
        return rv;

    api_ctxt.api_params = api::api_params_alloc(api::OBJ_ID_SUBNET, op);
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = op;
        api_ctxt.obj_id = api::OBJ_ID_SUBNET;
        if (op == api::API_OP_DELETE)
            api_ctxt.api_params->subnet_key = *key;
        else
            api_ctxt.api_params->subnet_info = *spec;
        return (api::g_api_engine.process_api(&api_ctxt));
    }
    return sdk::SDK_RET_OOM;
}

static inline sdk::sdk_ret_t
oci_subnet_stats_fill (subnet_entry *entry, oci_subnet_stats_t *stats)
{
    return SDK_RET_OK;
}

static inline sdk::sdk_ret_t
oci_subnet_status_fill (subnet_entry *entry, oci_subnet_status_t *status)
{
    status->hw_id = entry->hw_id();
    return SDK_RET_OK;
}

static inline sdk::sdk_ret_t
oci_subnet_spec_fill (subnet_entry *entry, oci_subnet_spec_t *spec)
{
    spec->v4_route_table = entry->v4_route_table();
    memcpy(&spec->vr_mac, entry->vr_mac(), sizeof(mac_addr_t));
    return SDK_RET_OK;
}

static inline subnet_entry *
oci_subnet_entry_find (oci_subnet_key_t *key)
{
    return (subnet_db()->subnet_find(key));
}

//----------------------------------------------------------------------------
// Subnet API entry point implementation
//----------------------------------------------------------------------------

sdk::sdk_ret_t
oci_subnet_create (oci_subnet_spec_t *spec)
{
    return (oci_subnet_api_handle(api::API_OP_CREATE, NULL, spec));
}

sdk::sdk_ret_t
oci_subnet_read (oci_subnet_key_t *key, oci_subnet_info_t *info)
{
    sdk::sdk_ret_t rv;
    subnet_entry *entry = NULL;

    if (key == NULL || info == NULL)
        return sdk::SDK_RET_INVALID_ARG;

    if ((entry = oci_subnet_entry_find(key)) == NULL)
        return sdk::SDK_RET_ENTRY_NOT_FOUND;

    if ((rv = oci_subnet_spec_fill(entry, &info->spec)) != sdk::SDK_RET_OK)
        return rv;

    if ((rv = oci_subnet_status_fill(entry, &info->status)) != sdk::SDK_RET_OK)
        return rv;

    if ((rv = oci_subnet_stats_fill(entry, &info->stats)) != sdk::SDK_RET_OK)
        return rv;

    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
oci_subnet_update (oci_subnet_spec_t *spec)
{
    return (oci_subnet_api_handle(api::API_OP_UPDATE, NULL, spec));
}

sdk::sdk_ret_t
oci_subnet_delete (oci_subnet_key_t *key)
{
    return (oci_subnet_api_handle(api::API_OP_DELETE, key, NULL));
}
