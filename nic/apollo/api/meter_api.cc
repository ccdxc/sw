//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file implements Meter CRUD API
///
//----------------------------------------------------------------------------

#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/obj_api.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/meter.hpp"
#include "nic/apollo/api/meter_state.hpp"

static sdk_ret_t
pds_meter_api_handle (api::api_op_t op, pds_meter_key_t *key,
                      pds_meter_spec_t *spec)
{
    sdk_ret_t rv;
    api_ctxt_t api_ctxt;

    if ((rv = pds_obj_api_validate(op, key, spec)) != SDK_RET_OK) {
        return rv;
    }

    api_ctxt.api_params = api::api_params_alloc(api::OBJ_ID_METER, op);
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = op;
        api_ctxt.obj_id = api::OBJ_ID_METER;
        if (op == api::API_OP_DELETE) {
            api_ctxt.api_params->meter_key = *key;
        } else {
            api_ctxt.api_params->meter_spec = *spec;
        }
        return (api::g_api_engine.process_api(&api_ctxt));
    }
    return SDK_RET_OOM;
}

static inline sdk_ret_t
pds_meter_stats_fill (meter_entry *entry, pds_meter_stats_t *stats)
{
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_meter_status_fill (meter_entry *entry, pds_meter_status_t *status)
{
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_meter_spec_fill (meter_entry *entry, pds_meter_spec_t *spec)
{
    // nothing to fill for meter
    return SDK_RET_OK;
}

static inline meter_entry *
pds_meter_entry_find (pds_meter_key_t *key)
{
    return (meter_db()->find(key));
}

//----------------------------------------------------------------------------
// Meter API entry point implementation
//----------------------------------------------------------------------------

sdk_ret_t
pds_meter_create (pds_meter_spec_t *spec)
{
    return (pds_meter_api_handle(api::API_OP_CREATE, NULL, spec));
}

sdk_ret_t
pds_meter_read (pds_meter_key_t *key, pds_meter_info_t *info)
{
    sdk_ret_t rv;
    meter_entry *entry = NULL;

    if (key == NULL || info == NULL) {
        return SDK_RET_INVALID_ARG;
    }

    if ((entry = pds_meter_entry_find(key)) == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    if ((rv = pds_meter_spec_fill(entry, &info->spec)) != SDK_RET_OK) {
        return rv;
    }

    if ((rv = pds_meter_status_fill(entry, &info->status)) != SDK_RET_OK) {
        return rv;
    }

    if ((rv = pds_meter_stats_fill(entry, &info->stats)) != SDK_RET_OK) {
        return rv;
    }

    return SDK_RET_OK;
}

sdk_ret_t
pds_meter_update (pds_meter_spec_t *spec)
{
    return (pds_meter_api_handle(api::API_OP_UPDATE, NULL, spec));
}

sdk_ret_t
pds_meter_delete (pds_meter_key_t *key)
{
    return (pds_meter_api_handle(api::API_OP_DELETE, key, NULL));
}
