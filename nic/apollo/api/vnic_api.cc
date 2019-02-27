//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module implements VNIC API
///
//----------------------------------------------------------------------------
#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/obj_api.hpp"
#include "nic/apollo/api/vnic.hpp"
#include "nic/apollo/api/impl/vnic_impl.hpp"
#include "nic/apollo/api/oci_state.hpp"

static sdk_ret_t
oci_vnic_api_handle (api::api_op_t op, oci_vnic_key_t *key,
                     oci_vnic_spec_t *spec)
{
    sdk::sdk_ret_t rv;
    api_ctxt_t api_ctxt;

    if ((rv = oci_obj_api_validate(op, key, spec)) != sdk::SDK_RET_OK)
        return rv;

    api_ctxt.api_params = api::api_params_alloc(api::OBJ_ID_VNIC, op);
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = op;
        api_ctxt.obj_id = api::OBJ_ID_VNIC;
        if (op == api::API_OP_DELETE)
            api_ctxt.api_params->vnic_key = *key;
        else
            api_ctxt.api_params->vnic_info = *spec;
        rv = api::g_api_engine.process_api(&api_ctxt);
        return rv;
    }
    return sdk::SDK_RET_OOM;
}

static inline vnic_entry *
oci_vnic_entry_find (oci_vnic_key_t *key)
{
    return (vnic_db()->vnic_find(key));
}

//----------------------------------------------------------------------------
// VNIC API entry point implementation
//----------------------------------------------------------------------------

sdk_ret_t
oci_vnic_create (oci_vnic_spec_t *spec)
{
    return (oci_vnic_api_handle(api::API_OP_CREATE, NULL, spec));
}

sdk::sdk_ret_t
oci_vnic_read (oci_vnic_key_t *key, oci_vnic_info_t *info)
{
    vnic_entry *entry = NULL;
    api::impl::vnic_impl *impl;

    if (key == NULL || info == NULL)
        return sdk::SDK_RET_INVALID_ARG;

    if ((entry = oci_vnic_entry_find(key)) == NULL)
        return sdk::SDK_RET_ENTRY_NOT_FOUND;

    info->spec.key = *key;
    // Call the vnic hw implementaion directly
    impl = dynamic_cast<api::impl::vnic_impl*>(entry->impl());
    return impl->read_hw(key, info);
}

sdk_ret_t
oci_vnic_update (oci_vnic_spec_t *spec)
{
    return (oci_vnic_api_handle(api::API_OP_UPDATE, NULL, spec));
}

sdk_ret_t
oci_vnic_delete (oci_vnic_key_t *key)
{
    return (oci_vnic_api_handle(api::API_OP_UPDATE, key, NULL));
}
