//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file implements interface CRUD API
///
//----------------------------------------------------------------------------

#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_msg.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/obj_api.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/if.hpp"
#include "nic/apollo/api/if_state.hpp"

static sdk_ret_t
pds_if_api_handle (pds_batch_ctxt_t bctxt, api::api_op_t op,
                   pds_if_key_t *key, pds_if_spec_t *spec)
{
    sdk_ret_t rv;
    api_ctxt_t *api_ctxt;

    if ((rv = pds_obj_api_validate(op, key, spec)) != SDK_RET_OK) {
        return rv;
    }

    api_ctxt = api::api_ctxt_alloc(api::OBJ_ID_IF, op);
    if (likely(api_ctxt != NULL)) {
        if (op == api::API_OP_DELETE) {
            api_ctxt->api_params->if_key = *key;
        } else {
            api_ctxt->api_params->if_spec = *spec;
        }
        return process_api(bctxt, api_ctxt);
    }
    return SDK_RET_OOM;
}

static inline sdk_ret_t
pds_if_stats_fill (pds_if_stats_t *stats, if_entry *entry)
{
    return SDK_RET_ERR;
}

static inline sdk_ret_t
pds_if_status_fill (pds_if_status_t *status, if_entry *entry)
{
    return SDK_RET_ERR;
}

static inline sdk_ret_t
pds_if_spec_fill (pds_if_spec_t *spec, if_entry *entry)
{
    return SDK_RET_ERR;
}

static inline if_entry *
pds_if_entry_find (pds_if_key_t *key)
{
    return (if_db()->find(key));
}

//----------------------------------------------------------------------------
// interface API entry point implementation
//----------------------------------------------------------------------------

sdk_ret_t
pds_if_create (_In_ pds_if_spec_t *spec, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_if_api_handle(bctxt, api::API_OP_CREATE, NULL, spec);
}

sdk_ret_t
pds_if_read (pds_if_key_t *key, pds_if_info_t *info)
{
    sdk_ret_t rv;
    if_entry *entry = NULL;

    if (key == NULL || info == NULL) {
        return SDK_RET_INVALID_ARG;
    }

    if ((entry = pds_if_entry_find(key)) == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    if ((rv = pds_if_spec_fill(&info->spec, entry)) != SDK_RET_OK) {
        return rv;
    }
    info->spec.key = *key;

    if ((rv = pds_if_status_fill(&info->status, entry)) != SDK_RET_OK) {
        return rv;
    }

    if ((rv = pds_if_stats_fill(&info->stats, entry)) != SDK_RET_OK) {
        return rv;
    }

    return SDK_RET_OK;
}

sdk_ret_t
pds_if_update (_In_ pds_if_spec_t *spec, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_if_api_handle(bctxt, api::API_OP_UPDATE, NULL, spec);
}

sdk_ret_t
pds_if_delete (_In_ pds_if_key_t *key, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_if_api_handle(bctxt, api::API_OP_DELETE, key, NULL);
}
