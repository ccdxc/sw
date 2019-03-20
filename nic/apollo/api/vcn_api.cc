//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file implements VCN CRUD API
///
//----------------------------------------------------------------------------

#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/obj_api.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/vcn.hpp"
#include "nic/apollo/api/vcn_state.hpp"

static sdk_ret_t
pds_vcn_api_handle (api::api_op_t op, pds_vcn_key_t *key, pds_vcn_spec_t *spec)
{
    sdk::sdk_ret_t rv;
    api_ctxt_t api_ctxt;

    if ((rv = pds_obj_api_validate(op, key, spec)) != sdk::SDK_RET_OK)
        return rv;

    api_ctxt.api_params = api::api_params_alloc(api::OBJ_ID_VCN, op);
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = op;
        api_ctxt.obj_id = api::OBJ_ID_VCN;
        if (op == api::API_OP_DELETE)
            api_ctxt.api_params->vcn_key = *key;
        else
            api_ctxt.api_params->vcn_spec = *spec;
        return (api::g_api_engine.process_api(&api_ctxt));
    }

    return sdk::SDK_RET_OOM;
}

static inline sdk::sdk_ret_t
pds_vcn_stats_fill (vcn_entry *entry, pds_vcn_stats_t *stats)
{
    return SDK_RET_OK;
}

static inline sdk::sdk_ret_t
pds_vcn_status_fill (vcn_entry *entry, pds_vcn_status_t *status)
{
    status->hw_id = entry->hw_id();
    return sdk::SDK_RET_OK;
}

static inline sdk::sdk_ret_t
pds_vcn_spec_fill (vcn_entry *entry, pds_vcn_spec_t *spec)
{
    return SDK_RET_OK;
}

static inline vcn_entry *
pds_vcn_entry_find (pds_vcn_key_t *key)
{
    return (vcn_db()->find(key));
}

//----------------------------------------------------------------------------
// VCN API entry point implementation
//----------------------------------------------------------------------------

sdk::sdk_ret_t
pds_vcn_create (pds_vcn_spec_t *spec)
{
    return (pds_vcn_api_handle(api::API_OP_CREATE, NULL, spec));
}

sdk::sdk_ret_t
pds_vcn_read (pds_vcn_key_t *key, pds_vcn_info_t *info)
{
    sdk::sdk_ret_t rv;
    vcn_entry *entry = NULL;

    if (key == NULL || info == NULL)
        return sdk::SDK_RET_INVALID_ARG;

    if ((entry = pds_vcn_entry_find(key)) == NULL) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }

    if ((rv = pds_vcn_spec_fill(entry, &info->spec)) != sdk::SDK_RET_OK)
        return rv;

    if ((rv = pds_vcn_status_fill(entry, &info->status)) != sdk::SDK_RET_OK)
        return rv;

    if ((rv = pds_vcn_stats_fill(entry, &info->stats)) != sdk::SDK_RET_OK)
        return rv;

    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
pds_vcn_update (pds_vcn_spec_t *spec)
{
    return (pds_vcn_api_handle(api::API_OP_UPDATE, NULL, spec));
}

sdk::sdk_ret_t
pds_vcn_delete (pds_vcn_key_t *key)
{
    return (pds_vcn_api_handle(api::API_OP_DELETE, key, NULL));
}
