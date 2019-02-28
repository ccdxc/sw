/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    route_api.cc
 *
 * @brief   route table handling
 */

#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/include/api/pds_route.hpp"
/**
 * @defgroup PDS_ROUTE_TABLE_API - first level of route table API handling
 * @ingroup PDS_ROUTE
 * @{
 */

/**
 * @brief create a route table
 *
 * @param[in] spec route table configuration
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
pds_route_table_create (_In_ pds_route_table_spec_t *spec)
{
    api_ctxt_t    api_ctxt;
    sdk_ret_t     rv;

    api_ctxt.api_params = api_params_alloc(api::OBJ_ID_ROUTE_TABLE,
                                           api::API_OP_CREATE);
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_CREATE;
        api_ctxt.obj_id = api::OBJ_ID_ROUTE_TABLE;
        api_ctxt.api_params->route_table_spec = *spec;
        rv = api::g_api_engine.process_api(&api_ctxt);
        return rv;
    }
    return sdk::SDK_RET_OOM;
}

/**
 * @brief delete given route table
 *
 * @param[in] route_talbe_key    route table key
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
pds_route_table_delete (_In_ pds_route_table_key_t *route_table_key)
{
    api_ctxt_t    api_ctxt;
    sdk_ret_t     rv;

    api_ctxt.api_params = api_params_alloc(api::OBJ_ID_ROUTE_TABLE,
                                           api::API_OP_DELETE);
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_DELETE;
        api_ctxt.obj_id = api::OBJ_ID_ROUTE_TABLE;
        api_ctxt.api_params->route_table_key = *route_table_key;
        rv = api::g_api_engine.process_api(&api_ctxt);
        return rv;
    }
    return sdk::SDK_RET_OOM;
}

/** @} */ // end of PDS_ROUTE_TABLE_API
