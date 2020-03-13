//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file implements TEP CRUD APIs
///
//----------------------------------------------------------------------------

#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_msg.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/tep.hpp"

static inline sdk_ret_t
pds_tep_api_handle (pds_batch_ctxt_t bctxt, api_op_t api_op, pds_obj_key_t *key,
                    pds_tep_spec_t *spec)
{
    api_ctxt_t *api_ctxt;

    api_ctxt = api::api_ctxt_alloc(OBJ_ID_TEP, api_op);
    if (likely(api_ctxt != NULL)) {
        if (api_op == API_OP_DELETE) {
            api_ctxt->api_params->tep_key = *key;
        } else {
            api_ctxt->api_params->tep_spec = *spec;
        }
        return  process_api(bctxt, api_ctxt);
    }
    return SDK_RET_OOM;
}

static inline tep_entry *
pds_tep_entry_find (pds_obj_key_t *key)
{
    return (tep_db()->find(key));
}

sdk_ret_t
pds_tep_create (_In_ pds_tep_spec_t *spec, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_tep_api_handle(bctxt, API_OP_CREATE, NULL, spec);
}

sdk_ret_t
pds_tep_read (_In_ pds_obj_key_t *key, _Out_ pds_tep_info_t *info)
{
    tep_entry *entry;

    if ((key == NULL) || (info == NULL)) {
        return sdk::SDK_RET_INVALID_ARG;
    }

    if ((entry = pds_tep_entry_find(key)) == NULL) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }

    return entry->read(info);
}

typedef struct pds_tep_read_args_s {
    tep_read_cb_t cb;
    void *ctxt;
} pds_tep_read_args_t;

static bool
pds_tep_info_from_entry (void *entry, void *ctxt)
{
    tep_entry *tep = (tep_entry *)entry;
    pds_tep_read_args_t *args = (pds_tep_read_args_t *)ctxt;
    pds_tep_info_t info;

    memset(&info, 0, sizeof(pds_tep_info_t));

    // call entry read
    tep->read(&info);

    // call cb on info
    args->cb(&info, args->ctxt);

    return false;
}

sdk_ret_t
pds_tep_read_all (tep_read_cb_t tep_read_cb, void *ctxt)
{
    pds_tep_read_args_t args = {0};

    args.ctxt = ctxt;
    args.cb = tep_read_cb;

    return tep_db()->walk(pds_tep_info_from_entry, &args);
}

sdk_ret_t
pds_tep_update (_In_ pds_tep_spec_t *spec, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_tep_api_handle(bctxt, API_OP_UPDATE, NULL, spec);
}

sdk_ret_t
pds_tep_delete (_In_ pds_obj_key_t *key, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_tep_api_handle(bctxt, API_OP_DELETE, key, NULL);
}
