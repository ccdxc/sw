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
#include "nic/apollo/api/utils.hpp"
#include <malloc.h>

//----------------------------------------------------------------------------
// route table API implementation entry point
//----------------------------------------------------------------------------

static sdk_ret_t
pds_route_table_api_handle (pds_batch_ctxt_t bctxt, api_op_t op,
                            pds_obj_key_t *key,
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
            api_ctxt->api_params->key = *key;
        } else {
            api_ctxt->api_params->route_table_spec = *spec;
            if (spec->route_info == NULL) {
                PDS_TRACE_ERR("Rejected route table %s, api op %u with no "
                              "routes", spec->key.str(), op);
                return SDK_RET_INVALID_ARG;
            }
        }
        return process_api(bctxt, api_ctxt);
    }
    return SDK_RET_OOM;
}

static inline route_table *
pds_route_table_find (pds_obj_key_t *key)
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
pds_route_table_read (_In_ pds_obj_key_t *key,
                      _Out_ pds_route_table_info_t *info)
{
    route_table *entry;

    if (key == NULL || info == NULL) {
        return SDK_RET_INVALID_ARG;
    }

    if ((entry = pds_route_table_find(key)) == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    return entry->read(info);
}

typedef struct pds_route_table_read_args_s {
    route_table_read_cb_t cb;
    void *ctxt;
} pds_route_table_read_args_t;

bool
pds_route_table_info_from_entry (void *entry, void *ctxt)
{
    route_table *route = (route_table *)entry;
    pds_route_table_read_args_t *args = (pds_route_table_read_args_t *)ctxt;
    pds_route_table_info_t info;
    uint32_t num_routes = 0;

    memset(&info, 0, sizeof(pds_route_table_info_t));

    // read number of routes and allocate memory for routes
    num_routes = route->num_routes();
    info.spec.route_info =
        (route_info_t *)SDK_CALLOC(PDS_MEM_ALLOC_ID_ROUTE_TABLE,
                                   ROUTE_INFO_SIZE(num_routes));
    info.spec.route_info->num_routes = num_routes;

    // entry read
    route->read(&info);

    // call cb on info
    args->cb(&info, args->ctxt);

    // free memory
    SDK_FREE(PDS_MEM_ALLOC_ID_ROUTE_TABLE, info.spec.route_info);
    info.spec.route_info = NULL;

    return false;
}

sdk_ret_t
pds_route_table_read_all (route_table_read_cb_t route_table_read_cb, void *ctxt)
{
    sdk_ret_t ret;
    pds_route_table_read_args_t args = {0};

    args.ctxt = ctxt;
    args.cb = route_table_read_cb;
    ret = route_table_db()->walk(pds_route_table_info_from_entry, &args);

    PDS_MEMORY_TRIM();
    return ret;
}

sdk_ret_t
pds_route_table_update (_In_ pds_route_table_spec_t *spec,
                        _In_ pds_batch_ctxt_t bctxt)
{
    return pds_route_table_api_handle(bctxt, API_OP_UPDATE, NULL, spec);
}

sdk_ret_t
pds_route_table_delete (_In_ pds_obj_key_t *key,
                        _In_ pds_batch_ctxt_t bctxt)
{
    return pds_route_table_api_handle(bctxt, API_OP_DELETE, key, NULL);
}

//----------------------------------------------------------------------------
// route API implementation entry point
//----------------------------------------------------------------------------

static inline sdk_ret_t
pds_route_api_handle (pds_batch_ctxt_t bctxt, api_op_t op,
                      pds_route_key_t *key, pds_route_spec_t *spec)
{
    sdk_ret_t rv;
    api_ctxt_t *api_ctxt;

    if ((rv = pds_obj_api_validate(op, key, spec)) != SDK_RET_OK) {
        return rv;
    }

    api_ctxt = api::api_ctxt_alloc(OBJ_ID_ROUTE, op);
    if (likely(api_ctxt != NULL)) {
        if (op == API_OP_DELETE) {
            api_ctxt->api_params->route_key = *key;
        } else {
            api_ctxt->api_params->route_spec = *spec;
        }
        return process_api(bctxt, api_ctxt);
    }
    return SDK_RET_OOM;
}

sdk_ret_t
pds_route_create (_In_ pds_route_spec_t *spec, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_route_api_handle(bctxt, API_OP_CREATE, NULL, spec);
}

sdk_ret_t
pds_route_read (_In_ pds_route_key_t *key, _Out_ pds_route_info_t *info)
{
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
pds_route_update (_In_ pds_route_spec_t *spec, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_route_api_handle(bctxt, API_OP_UPDATE, NULL, spec);
}

sdk_ret_t
pds_route_delete (_In_ pds_route_key_t *key, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_route_api_handle(bctxt, API_OP_DELETE, key, NULL);
}
