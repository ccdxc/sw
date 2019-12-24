//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file implements VNIC CRUD APIs
///
//----------------------------------------------------------------------------

#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_msg.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/obj_api.hpp"
#include "nic/apollo/api/vnic.hpp"
#include "nic/apollo/api/pds_state.hpp"

static inline sdk_ret_t
pds_vnic_api_handle (pds_batch_ctxt_t bctxt, api_op_t op,
                     pds_vnic_key_t *key, pds_vnic_spec_t *spec)
{
    sdk_ret_t rv;
    api_ctxt_t *api_ctxt;

    if ((rv = pds_obj_api_validate(op, key, spec)) != SDK_RET_OK) {
        return rv;
    }

    api_ctxt = api::api_ctxt_alloc(OBJ_ID_VNIC, op);
    if (likely(api_ctxt != NULL)) {
        if (op == API_OP_DELETE) {
            api_ctxt->api_params->vnic_key = *key;
        } else {
            api_ctxt->api_params->vnic_spec = *spec;
        }
        return process_api(bctxt, api_ctxt);
    }
    return SDK_RET_OOM;
}

static inline vnic_entry *
pds_vnic_entry_find (pds_vnic_key_t *key)
{
    return (vnic_db()->find(key));
}

//----------------------------------------------------------------------------
// VNIC API entry point implementation
//----------------------------------------------------------------------------
sdk_ret_t
pds_vnic_create (_In_ pds_vnic_spec_t *spec, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_vnic_api_handle(bctxt, API_OP_CREATE, NULL, spec);
}

sdk_ret_t
pds_vnic_read (_In_ pds_vnic_key_t *key, _Out_ pds_vnic_info_t *info)
{
    vnic_entry *entry;

    if (key == NULL || info == NULL) {
        return SDK_RET_INVALID_ARG;
    }

    if ((entry = pds_vnic_entry_find(key)) == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    return entry->read(info);
}

sdk_ret_t
pds_vnic_update (_In_ pds_vnic_spec_t *spec, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_vnic_api_handle(bctxt, API_OP_UPDATE, NULL, spec);
}

sdk_ret_t
pds_vnic_delete (_In_ pds_vnic_key_t *key, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_vnic_api_handle(bctxt, API_OP_DELETE, key, NULL);
}
