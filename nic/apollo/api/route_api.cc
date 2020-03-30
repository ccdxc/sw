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
    sdk_ret_t ret;
    route_table *entry;

    if (key == NULL || info == NULL) {
        return SDK_RET_INVALID_ARG;
    }

    if ((entry = pds_route_table_find(key)) == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    if (info->spec.route_info == NULL) {
        return entry->read(info);
    }

    if (!info->spec.route_info->num_routes) {
        info->spec.route_info->num_routes = entry->num_routes();
    } else {
        uint32_t num_routes_to_read = info->spec.route_info->num_routes;
        if (num_routes_to_read < entry->num_routes()) {
            ret = entry->read(info);
            if (ret != SDK_RET_OK) {
                return ret;
            }
            // buffer is smaller, read all routes and copy over the
            // requested number allocate memory for reading all the routes
            route_info_t *route_info =
                (route_info_t *)SDK_CALLOC(api::PDS_MEM_ALLOC_ID_ROUTE_TABLE,
                                           ROUTE_INFO_SIZE(entry->num_routes()));
            route_info->num_routes = entry->num_routes();
            // retrieve all routes
            ret = route_table_db()->retrieve_routes(key, route_info);
            if (ret != SDK_RET_OK) {
                SDK_FREE(api::PDS_MEM_ALLOC_ID_ROUTE_TABLE, route_info);
                return ret;
            }
            // copy over requested number of routes
            memcpy(info->spec.route_info, route_info,
                   ROUTE_INFO_SIZE(num_routes_to_read));
            info->spec.route_info->num_routes = num_routes_to_read;
            // free allocated memory
            SDK_FREE(api::PDS_MEM_ALLOC_ID_ROUTE_TABLE, route_info);
        } else {
            ret = entry->read(info);
            if (ret != SDK_RET_OK) {
                return ret;
            }
            // read route table entries from lmdb
            return route_table_db()->retrieve_routes(key,
                                                     info->spec.route_info);
        }
    }
    return SDK_RET_OK;
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
                      pds_obj_key_t *key, pds_route_spec_t *spec)
{
    sdk_ret_t rv;
    api_ctxt_t *api_ctxt;

    if ((rv = pds_obj_api_validate(op, key, spec)) != SDK_RET_OK) {
        return rv;
    }

    api_ctxt = api::api_ctxt_alloc(OBJ_ID_ROUTE, op);
    if (likely(api_ctxt != NULL)) {
        if (op == API_OP_DELETE) {
            api_ctxt->api_params->key = *key;
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
pds_route_read (_In_ pds_obj_key_t *key, _Out_ pds_policy_info_t *info)
{
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
pds_route_update (_In_ pds_route_spec_t *spec, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_route_api_handle(bctxt, API_OP_UPDATE, NULL, spec);
}

sdk_ret_t
pds_route_delete (_In_ pds_obj_key_t *key, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_route_api_handle(bctxt, API_OP_DELETE, key, NULL);
}
