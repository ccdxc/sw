//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file implements route table CRUD APIs
///
//----------------------------------------------------------------------------

#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_msg.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/include/pds_route.hpp"
#include "nic/apollo/api/obj_api.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/route.hpp"

static sdk_ret_t
pds_route_table_api_handle (pds_batch_ctxt_t bctxt, api_op_t op,
                            pds_route_table_key_t *key,
                            pds_route_table_spec_t *spec)
{
    sdk_ret_t rv;
    api_ctxt_t *api_ctxt;

    if ((rv = pds_obj_api_validate(op, key, spec)) != SDK_RET_OK) {
        return rv;
    }

    api_ctxt = api::api_ctxt_alloc(OBJ_ID_ROUTE_TABLE, op);
    if (likely(api_ctxt != NULL)) {
        if (op == API_OP_DELETE) {
            api_ctxt->api_params->route_table_key = *key;
        } else {
            api_ctxt->api_params->route_table_spec = *spec;
        }
        return process_api(bctxt, api_ctxt);
    }
    return SDK_RET_OOM;
}

static inline route_table *
pds_route_table_find (pds_route_table_key_t *key)
{
    return (route_table_db()->find(key));
}

sdk_ret_t
pds_route_table_create (_In_ pds_route_table_spec_t *spec,
                        _In_ pds_batch_ctxt_t bctxt)
{
    return pds_route_table_api_handle(bctxt, API_OP_CREATE, NULL, spec);
}

sdk_ret_t
pds_route_table_read (_In_ pds_route_table_key_t *key,
                      _Out_ pds_route_table_info_t *info)
{
    route_table *entry;

    if (key == NULL || info == NULL) {
        return sdk::SDK_RET_INVALID_ARG;
    }

    if ((entry = pds_route_table_find(key)) == NULL) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }

    return entry->read(info);
}

sdk_ret_t
pds_route_table_update (_In_ pds_route_table_spec_t *spec,
                        _In_ pds_batch_ctxt_t bctxt)
{
    return pds_route_table_api_handle(bctxt, API_OP_UPDATE, NULL, spec);
}

sdk_ret_t
pds_route_table_delete (_In_ pds_route_table_key_t *key,
                        _In_ pds_batch_ctxt_t bctxt)
{
    return pds_route_table_api_handle(bctxt, API_OP_DELETE, key, NULL);
}
