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
#include "nic/apollo/api/oci_state.hpp"
#include "nic/apollo/api/tep.hpp"
#include "nic/apollo/api/impl/tep_impl.hpp"

static inline sdk_ret_t
oci_tep_api_handle (api_op_t api_op, oci_tep_key_t *key, oci_tep_spec_t *spec)
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
oci_tep_create (oci_tep_spec_t *spec)
{
    return (oci_tep_api_handle(api::API_OP_CREATE, NULL, spec));
}

sdk_ret_t
oci_tep_read (oci_tep_key_t *key, oci_tep_info_t *info)
{
    tep_entry *entry;
    api::impl::tep_impl *impl;

    if ((key == NULL) || (info == NULL)) {
        return sdk::SDK_RET_INVALID_ARG;
    }

    if ((entry = tep_entry::find_in_db(key)) == NULL) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }

    info->spec.key = *key;
    impl = dynamic_cast<api::impl::tep_impl*>(entry->impl());
    return impl->read_hw(info);
}

sdk_ret_t
oci_tep_update (oci_tep_spec_t *spec)
{
    return (oci_tep_api_handle(api::API_OP_UPDATE, NULL, spec));
}

sdk_ret_t
oci_tep_delete (oci_tep_key_t *key)
{
    return (oci_tep_api_handle(api::API_OP_DELETE, key, NULL));
}
