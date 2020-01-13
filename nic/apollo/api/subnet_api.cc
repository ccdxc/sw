//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file implements subnet CRUD APIs
///
//----------------------------------------------------------------------------

#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_msg.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/obj_api.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/subnet.hpp"
#include "nic/apollo/api/subnet_state.hpp"

static sdk_ret_t
pds_subnet_api_handle (pds_batch_ctxt_t bctxt, api_op_t op,
                       pds_obj_key_t *key, pds_subnet_spec_t *spec)
{
    sdk_ret_t rv;
    api_ctxt_t *api_ctxt;

    if ((rv = pds_obj_api_validate(op, key, spec)) != SDK_RET_OK) {
        return rv;
    }

    // allocate API context
    api_ctxt = api::api_ctxt_alloc(OBJ_ID_SUBNET, op);
    if (likely(api_ctxt != NULL)) {
        if (op == API_OP_DELETE) {
            api_ctxt->api_params->subnet_key = *key;
        } else {
            api_ctxt->api_params->subnet_spec = *spec;
        }
        return process_api(bctxt, api_ctxt);
    }
    return SDK_RET_OOM;
}

static inline subnet_entry *
pds_subnet_entry_find (pds_obj_key_t *key)
{
    return (subnet_db()->find(key));
}

//----------------------------------------------------------------------------
// Subnet API entry point implementation
//----------------------------------------------------------------------------

sdk_ret_t
pds_subnet_create (_In_ pds_subnet_spec_t *spec, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_subnet_api_handle(bctxt, API_OP_CREATE, NULL, spec);
}

sdk_ret_t
pds_subnet_read (_In_ pds_obj_key_t *key, _Out_ pds_subnet_info_t *info)
{
    subnet_entry *entry;

    if (key == NULL || info == NULL) {
        return sdk::SDK_RET_INVALID_ARG;
    }

    if ((entry = pds_subnet_entry_find(key)) == NULL) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }

    return entry->read(info);
}

sdk_ret_t
pds_subnet_update (_In_ pds_subnet_spec_t *spec, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_subnet_api_handle(bctxt, API_OP_UPDATE, NULL, spec);
}

sdk_ret_t
pds_subnet_delete (_In_ pds_obj_key_t *key, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_subnet_api_handle(bctxt, API_OP_DELETE, key, NULL);
}
