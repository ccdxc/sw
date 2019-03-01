//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module implements Mapping API
///
//----------------------------------------------------------------------------
#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/obj_api.hpp"
#include "nic/apollo/api/impl/mapping_impl.hpp"
#include "nic/apollo/api/mapping.hpp"

static sdk_ret_t
pds_mapping_api_handle (api::api_op_t op, pds_mapping_key_t *key,
                        pds_mapping_spec_t *spec)
{
    sdk::sdk_ret_t rv;
    api_ctxt_t api_ctxt;

    if ((rv = pds_obj_api_validate(op, key, spec)) != sdk::SDK_RET_OK)
        return rv;

    api_ctxt.api_params = api::api_params_alloc(api::OBJ_ID_MAPPING, op);
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = op;
        api_ctxt.obj_id = api::OBJ_ID_MAPPING;
        if (op == api::API_OP_DELETE)
            api_ctxt.api_params->mapping_key = *key;
        else
            api_ctxt.api_params->mapping_spec = *spec;
        rv = api::g_api_engine.process_api(&api_ctxt);
        return rv;
    }
    return sdk::SDK_RET_OOM;
}

static inline mapping_entry *
pds_mapping_entry_find (pds_mapping_key_t *key)
{
    pds_mapping_spec_t spec = {0};
    spec.key = *key;
    // Mapping does not have any entry database
    // As the call are single thread, we can use static entry
    static mapping_entry *mapping;

    if (mapping == NULL) {
        mapping  = mapping_entry::factory(&spec);
    }
    return mapping;
}

//----------------------------------------------------------------------------
// Mapping API entry point implementation
//----------------------------------------------------------------------------

sdk_ret_t
pds_mapping_create (pds_mapping_spec_t *spec)
{
    return (pds_mapping_api_handle(api::API_OP_CREATE, NULL, spec));
}

sdk::sdk_ret_t
pds_mapping_read (pds_mapping_key_t *key, pds_mapping_info_t *info)
{
    mapping_entry *entry = NULL;
    api::impl::mapping_impl *impl;

    if (key == NULL || info == NULL)
        return sdk::SDK_RET_INVALID_ARG;

    if ((entry = pds_mapping_entry_find(key)) == NULL)
        return sdk::SDK_RET_ENTRY_NOT_FOUND;

    info->spec.key = *key;
    // Call the mapping hw implementaion directly
    impl = dynamic_cast<api::impl::mapping_impl*>(entry->impl());
    return impl->read_hw(key, info);
}

sdk_ret_t
pds_mapping_update (pds_mapping_spec_t *spec)
{
    return (pds_mapping_api_handle(api::API_OP_UPDATE, NULL, spec));
}

sdk_ret_t
pds_mapping_delete (pds_mapping_key_t *key)
{
    return (pds_mapping_api_handle(api::API_OP_DELETE, key, NULL));
}
