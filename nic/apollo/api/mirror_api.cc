//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module implements mirror session API
///
//----------------------------------------------------------------------------

#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/include/pds_mirror.hpp"
#include "nic/apollo/api/impl/mirror_impl.hpp"

sdk_ret_t
pds_mirror_session_create (_In_ pds_mirror_session_spec_t *spec)
{
    api_ctxt_t    api_ctxt;
    sdk_ret_t     rv;

    api_ctxt.api_params = api_params_alloc(api::OBJ_ID_MIRROR_SESSION,
                                           api::API_OP_CREATE);
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_CREATE;
        api_ctxt.obj_id = api::OBJ_ID_MIRROR_SESSION;
        api_ctxt.api_params->mirror_session_spec = *spec;
        rv = api::g_api_engine.process_api(&api_ctxt);
        return rv;
    }
    return SDK_RET_OOM;
}

sdk_ret_t
pds_mirror_session_delete (_In_ pds_mirror_session_key_t *key)
{
    api_ctxt_t    api_ctxt;
    sdk_ret_t     rv;

    api_ctxt.api_params = api_params_alloc(api::OBJ_ID_MIRROR_SESSION,
                                           api::API_OP_DELETE);
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_DELETE;
        api_ctxt.obj_id = api::OBJ_ID_MIRROR_SESSION;
        api_ctxt.api_params->mirror_session_key = *key;
        rv = api::g_api_engine.process_api(&api_ctxt);
        return rv;
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
pds_mirror_session_get (pds_mirror_session_key_t *key,
                        pds_mirror_session_info_t *info)
{
    mirror_session *entry = NULL;
    api::impl::mirror_impl *impl;

    if (key == NULL || info == NULL) {
        return sdk::SDK_RET_INVALID_ARG;
    }

    if ((entry = pds_mirror_session_find(key)) == NULL) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }
    info->spec.key = *key;

    // call the mirror hw implementaion directly
    impl = dynamic_cast<api::impl::mirror_impl*>(entry->impl());
    return impl->read_hw(key, info);
}
