//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file implements Meter CRUD APIs
///
//----------------------------------------------------------------------------

#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_msg.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/obj_api.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/meter.hpp"
#include "nic/apollo/api/meter_state.hpp"

static sdk_ret_t
pds_meter_api_handle (pds_batch_ctxt_t bctxt, api_op_t op,
                      pds_obj_key_t *key, pds_meter_spec_t *spec)
{
    sdk_ret_t rv;
    api_ctxt_t *api_ctxt;

    if ((rv = pds_obj_api_validate(op, key, spec)) != SDK_RET_OK) {
        return rv;
    }

    api_ctxt = api::api_ctxt_alloc(OBJ_ID_METER, op);
    if (likely(api_ctxt != NULL)) {
        if (op == API_OP_DELETE) {
            api_ctxt->api_params->key = *key;
        } else {
            api_ctxt->api_params->meter_spec = *spec;
        }
        return process_api(bctxt, api_ctxt);
    }
    return SDK_RET_OOM;
}

static inline meter_entry *
pds_meter_entry_find (pds_obj_key_t *key)
{
    return (meter_db()->find(key));
}

//----------------------------------------------------------------------------
// Meter API entry point implementation
//----------------------------------------------------------------------------

sdk_ret_t
pds_meter_create (_In_ pds_meter_spec_t *spec, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_meter_api_handle(bctxt, API_OP_CREATE, NULL, spec);
}

sdk_ret_t
pds_meter_read (_In_ pds_obj_key_t *key, _Out_ pds_meter_info_t *info)
{
    meter_entry *entry;

    if (key == NULL || info == NULL) {
        return SDK_RET_INVALID_ARG;
    }

    if ((entry = pds_meter_entry_find(key)) == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    return entry->read(info);
}

typedef struct pds_meter_read_args_s {
    meter_read_cb_t cb;
    void *ctxt;
} pds_meter_read_args_t;

bool
pds_meter_info_from_entry (void *entry, void *ctxt)
{
    meter_entry *meter = (meter_entry *)entry;
    pds_meter_read_args_t *args = (pds_meter_read_args_t *)ctxt;
    pds_meter_info_t info;

    memset(&info, 0, sizeof(pds_meter_info_t));

    // call entry read
    meter->read(&info);

    // call cb on info
    args->cb(&info, args->ctxt);
    
    return false;
}

sdk_ret_t
pds_meter_read_all (meter_read_cb_t meter_read_cb, void *ctxt)
{
    pds_meter_read_args_t args = {0};

    args.ctxt = ctxt;
    args.cb = meter_read_cb;

    return meter_db()->walk(pds_meter_info_from_entry, &args);
}

sdk_ret_t
pds_meter_update (_In_ pds_meter_spec_t *spec, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_meter_api_handle(bctxt, API_OP_UPDATE, NULL, spec);
}

sdk_ret_t
pds_meter_delete (_In_ pds_obj_key_t *key, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_meter_api_handle(bctxt, API_OP_DELETE, key, NULL);
}
