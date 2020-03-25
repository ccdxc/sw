//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file implements policer CRUD APIs
///
//----------------------------------------------------------------------------

#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_msg.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/obj_api.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/policer.hpp"
#include "nic/apollo/api/policer_state.hpp"

static sdk_ret_t
pds_policer_api_handle (pds_batch_ctxt_t bctxt, api_op_t op,
                        pds_obj_key_t *key, pds_policer_spec_t *spec)
{
    sdk_ret_t rv;
    api_ctxt_t *api_ctxt;

    if ((rv = pds_obj_api_validate(op, key, spec)) != SDK_RET_OK)
        return rv;

    // allocate API context
    api_ctxt = api::api_ctxt_alloc(OBJ_ID_POLICER, op);
    if (likely(api_ctxt != NULL)) {
        if (op == API_OP_DELETE) {
            api_ctxt->api_params->key = *key;
        } else {
            api_ctxt->api_params->policer_spec = *spec;
        }
        return process_api(bctxt, api_ctxt);
    }
    return SDK_RET_OOM;
}

static inline sdk_ret_t
pds_policer_stats_fill (pds_policer_stats_t *stats, policer_entry *entry)
{
    return SDK_RET_INVALID_OP;
}

static inline sdk_ret_t
pds_policer_status_fill (pds_policer_status_t *status, policer_entry *entry)
{
    return SDK_RET_INVALID_OP;
}

static inline sdk_ret_t
pds_policer_spec_fill (pds_policer_spec_t *spec, policer_entry *entry)
{
    return SDK_RET_INVALID_OP;
}

static inline policer_entry *
pds_policer_entry_find (pds_obj_key_t *key)
{
    return (policer_db()->find(key));
}

//----------------------------------------------------------------------------
// policer API entry point implementation
//----------------------------------------------------------------------------

sdk_ret_t
pds_policer_create (_In_ pds_policer_spec_t *spec, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_policer_api_handle(bctxt, API_OP_CREATE, NULL, spec);
}

sdk_ret_t
pds_policer_read (_In_ pds_obj_key_t *key, _Out_ pds_policer_info_t *info)
{
    policer_entry *entry;

    if (key == NULL || info == NULL) {
        return sdk::SDK_RET_INVALID_ARG;
    }

    if ((entry = pds_policer_entry_find(key)) == NULL) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }

    return entry->read(info);
}

typedef struct pds_policer_read_args_s {
    policer_read_cb_t cb;
    void *ctxt;
} pds_policer_read_args_t;

bool
pds_policer_info_from_entry (void *entry, void *ctxt)
{
    policer_entry *policer = (policer_entry *)entry;
    pds_policer_read_args_t *args = (pds_policer_read_args_t *)ctxt;
    pds_policer_info_t info = { 0 };

    // call entry read
    policer->read(&info);

    // call cb on info
    args->cb(&info, args->ctxt);

    return false;
}

sdk_ret_t
pds_policer_read_all (policer_read_cb_t cb, void *ctxt)
{
    pds_policer_read_args_t args = {0};

    args.ctxt = ctxt;
    args.cb = cb;

    return policer_db()->walk(pds_policer_info_from_entry, &args);
}

sdk_ret_t
pds_policer_update (_In_ pds_policer_spec_t *spec, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_policer_api_handle(bctxt, API_OP_UPDATE, NULL, spec);
}

sdk_ret_t
pds_policer_delete (_In_ pds_obj_key_t *key, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_policer_api_handle(bctxt, API_OP_DELETE, key, NULL);
}
