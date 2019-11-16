/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    route_api.cc
 *
 * @brief   route table handling
 */

#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_msg.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/include/pds_route.hpp"
#include "nic/apollo/api/obj_api.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/route.hpp"

/**
 * @defgroup PDS_ROUTE_TABLE_API - first level of route table API handling
 * @ingroup PDS_ROUTE
 * @{
 */

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

/**
 * @brief create a route table
 * @param[in] spec route table configuration
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
pds_route_table_create (_In_ pds_route_table_spec_t *spec,
                        _In_ pds_batch_ctxt_t bctxt)
{
    return pds_route_table_api_handle(bctxt, API_OP_CREATE, NULL, spec);
}

sdk::sdk_ret_t
pds_route_table_read (pds_route_table_key_t *key, pds_route_table_info_t *info)
{
    route_table *entry = NULL;

    if (key == NULL || info == NULL) {
        return sdk::SDK_RET_INVALID_ARG;
    }

    if ((entry = pds_route_table_find(key)) == NULL) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }
    return entry->read(info);
}

/**
 * @brief update a route table
 * @param[in] spec route table configuration
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
pds_route_table_update (_In_ pds_route_table_spec_t *spec,
                        _In_ pds_batch_ctxt_t bctxt)
{
    return pds_route_table_api_handle(bctxt, API_OP_UPDATE, NULL, spec);
}

/**
 * @brief delete given route table
 * @param[in] key    route table key
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
pds_route_table_delete (_In_ pds_route_table_key_t *key,
                        _In_ pds_batch_ctxt_t bctxt)
{
    return pds_route_table_api_handle(bctxt, API_OP_DELETE, key, NULL);
}

/** @} */    // end of PDS_ROUTE_TABLE_API
