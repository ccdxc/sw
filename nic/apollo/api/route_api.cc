/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    route_api.cc
 *
 * @brief   route table handling
 */

#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/include/pds_route.hpp"
#include "nic/apollo/api/obj_api.hpp"

/**
 * @defgroup PDS_ROUTE_TABLE_API - first level of route table API handling
 * @ingroup PDS_ROUTE
 * @{
 */

static sdk_ret_t
pds_route_table_api_handle (api::api_op_t op,
                            pds_route_table_key_t *key,
                            pds_route_table_spec_t *spec)
{
    sdk_ret_t rv;
    api_ctxt_t api_ctxt;

    if ((rv = pds_obj_api_validate(op, key, spec)) != SDK_RET_OK) {
        return rv;
    }

    api_ctxt.api_params = api::api_params_alloc(api::OBJ_ID_ROUTE_TABLE, op);
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = op;
        api_ctxt.obj_id = api::OBJ_ID_ROUTE_TABLE;
        if (op == api::API_OP_DELETE) {
            api_ctxt.api_params->route_table_key = *key;
        } else {
            api_ctxt.api_params->route_table_spec = *spec;
        }
        return (api::g_api_engine.process_api(&api_ctxt));
    }
    return SDK_RET_OOM;
}

/**
 * @brief create a route table
 * @param[in] spec route table configuration
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
pds_route_table_create (_In_ pds_route_table_spec_t *spec)
{
    return (pds_route_table_api_handle(api::API_OP_CREATE, NULL, spec));
}

/**
 * @brief update a route table
 * @param[in] spec route table configuration
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
pds_route_table_update (_In_ pds_route_table_spec_t *spec)
{
    return (pds_route_table_api_handle(api::API_OP_UPDATE, NULL, spec));
}

/**
 * @brief delete given route table
 * @param[in] key    route table key
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
pds_route_table_delete (_In_ pds_route_table_key_t *key)
{
    return (pds_route_table_api_handle(api::API_OP_DELETE, key, NULL));
}

/** @} */    // end of PDS_ROUTE_TABLE_API
