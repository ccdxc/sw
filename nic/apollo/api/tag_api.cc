//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file implements Tag CRUD APIs
///
//----------------------------------------------------------------------------

#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_msg.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/obj_api.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/tag.hpp"
#include "nic/apollo/api/tag_state.hpp"

static sdk_ret_t
pds_tag_api_handle (pds_batch_ctxt_t bctxt, api_op_t op,
                    pds_obj_key_t *key, pds_tag_spec_t *spec)
{
    sdk_ret_t rv;
    api_ctxt_t *api_ctxt;

    if ((rv = pds_obj_api_validate(op, key, spec)) != SDK_RET_OK) {
        return rv;
    }

    api_ctxt = api::api_ctxt_alloc(OBJ_ID_TAG, op);
    if (likely(api_ctxt != NULL)) {
        if (op == API_OP_DELETE) {
            api_ctxt->api_params->tag_key = *key;
        } else {
            api_ctxt->api_params->tag_spec = *spec;
        }
        return process_api(bctxt, api_ctxt);
    }
    return SDK_RET_OOM;
}

static inline tag_entry *
pds_tag_entry_find (pds_obj_key_t *key)
{
    return (tag_db()->find(key));
}

//----------------------------------------------------------------------------
// Tag API entry point implementation
//----------------------------------------------------------------------------

sdk_ret_t
pds_tag_create (_In_ pds_tag_spec_t *spec, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_tag_api_handle(bctxt, API_OP_CREATE, NULL, spec);
}

sdk_ret_t
pds_tag_read (_In_ pds_obj_key_t *key, _Out_ pds_tag_info_t *info)
{
    tag_entry *entry;

    if (key == NULL || info == NULL) {
        return SDK_RET_INVALID_ARG;
    }

    if ((entry = pds_tag_entry_find(key)) == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    return entry->read(info);
}

sdk_ret_t
pds_tag_update (_In_ pds_tag_spec_t *spec, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_tag_api_handle(bctxt, API_OP_UPDATE, NULL, spec);
}

sdk_ret_t
pds_tag_delete (_In_ pds_obj_key_t *key, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_tag_api_handle(bctxt, API_OP_DELETE, key, NULL);
}
