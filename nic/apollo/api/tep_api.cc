//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module implements TEP API
///
//----------------------------------------------------------------------------
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/tep.hpp"

static inline sdk_ret_t
pds_tep_api_handle (api_op_t api_op, pds_tep_key_t *key, pds_tep_spec_t *spec)
{
    api_ctxt_t api_ctxt;

    api_ctxt.api_params = api::api_params_alloc(api::OBJ_ID_TEP, api_op);
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api_op;
        api_ctxt.obj_id = api::OBJ_ID_TEP;
        if (api_op == api::API_OP_DELETE) {
            api_ctxt.api_params->tep_key = *key;
        } else {
            api_ctxt.api_params->tep_spec = *spec;
        }

        return (api::g_api_engine.process_api(&api_ctxt));
    }

    return sdk::SDK_RET_OOM;
}

sdk_ret_t
pds_tep_create (pds_tep_spec_t *spec)
{
    return (pds_tep_api_handle(api::API_OP_CREATE, NULL, spec));
}

sdk_ret_t
pds_tep_read (pds_tep_key_t *key, pds_tep_info_t *info)
{
    tep_entry *entry;

    if ((key == NULL) || (info == NULL)) {
        return sdk::SDK_RET_INVALID_ARG;
    }

    if ((entry = tep_db()->find(key)) == NULL) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }

    info->spec.key = *key;
    return entry->impl()->read_hw(NULL, (obj_info_t *)info);
}

sdk_ret_t
pds_tep_update (pds_tep_spec_t *spec)
{
    return (pds_tep_api_handle(api::API_OP_UPDATE, NULL, spec));
}

sdk_ret_t
pds_tep_delete (pds_tep_key_t *key)
{
    return (pds_tep_api_handle(api::API_OP_DELETE, key, NULL));
}
