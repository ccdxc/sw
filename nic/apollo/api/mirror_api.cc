//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module implements mirror session API
///
//----------------------------------------------------------------------------

#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_msg.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/include/pds_mirror.hpp"
#include "nic/apollo/api/obj_api.hpp"
#include "nic/apollo/api/mirror.hpp"

static sdk_ret_t
pds_mirror_session_api_handle (pds_batch_ctxt_t bctxt, api_op_t op,
                               pds_mirror_session_key_t *key,
                               pds_mirror_session_spec_t *spec)
{
    sdk_ret_t rv;
    api_ctxt_t *api_ctxt;

    if ((rv = pds_obj_api_validate(op, key, spec)) != SDK_RET_OK) {
        return rv;
    }

    api_ctxt = api::api_ctxt_alloc(OBJ_ID_MIRROR_SESSION, op);
    if (likely(api_ctxt != NULL)) {
        if (op == API_OP_DELETE) {
            api_ctxt->api_params->mirror_session_key = *key;
        } else {
            api_ctxt->api_params->mirror_session_spec = *spec;
        }
        return process_api(bctxt, api_ctxt);
    }
    return SDK_RET_OOM;
}

static inline mirror_session *
pds_mirror_session_find (pds_mirror_session_key_t *key)
{
    pds_mirror_session_spec_t spec = {0};
    spec.key = *key;
    static mirror_session *ms;

    if (ms == NULL) {
        ms = mirror_session::factory(&spec);
    }
    return ms;
}

sdk_ret_t
pds_mirror_session_create (_In_ pds_mirror_session_spec_t *spec,
                           _In_ pds_batch_ctxt_t bctxt)
{
    return pds_mirror_session_api_handle(bctxt, API_OP_CREATE, NULL, spec);
}

sdk_ret_t
pds_mirror_session_read (pds_mirror_session_key_t *key,
                        pds_mirror_session_info_t *info)
{
    mirror_session *entry = NULL;

    if (key == NULL || info == NULL) {
        return sdk::SDK_RET_INVALID_ARG;
    }

    if ((entry = pds_mirror_session_find(key)) == NULL) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }
    info->spec.key = *key;
    return entry->read(key, info);
}

sdk_ret_t
pds_mirror_session_update (_In_ pds_mirror_session_spec_t *spec,
                           _In_ pds_batch_ctxt_t bctxt)
{
    return pds_mirror_session_api_handle(bctxt, API_OP_UPDATE, NULL, spec);
}

sdk_ret_t
pds_mirror_session_delete (_In_ pds_mirror_session_key_t *key,
                           _In_ pds_batch_ctxt_t bctxt)
{
    return pds_mirror_session_api_handle(bctxt, API_OP_DELETE, key, NULL);
}
