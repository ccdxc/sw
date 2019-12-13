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
pds_tep_api_handle (pds_batch_ctxt_t bctxt, api_op_t api_op, pds_tep_key_t *key,
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
pds_tep_entry_find (pds_tep_key_t *key)
{
    return (tep_db()->find(key));
}

sdk_ret_t
pds_tep_create (_In_ pds_tep_spec_t *spec, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_tep_api_handle(bctxt, API_OP_CREATE, NULL, spec);
}

sdk_ret_t
pds_tep_read (_In_ pds_tep_key_t *key, _Out_ pds_tep_info_t *info)
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

sdk_ret_t
pds_tep_update (_In_ pds_tep_spec_t *spec, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_tep_api_handle(bctxt, API_OP_UPDATE, NULL, spec);
}

sdk_ret_t
pds_tep_delete (_In_ pds_tep_key_t *key, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_tep_api_handle(bctxt, API_OP_DELETE, key, NULL);
}
