/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    subnet_api.cc
 *
 * @brief   subnet API handling
 */

#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/subnet.hpp"

/**
 * @defgroup OCI_SUBNET_API - first level of subnet API handling
 * @ingroup OCI_SUBNET
 * @{
 */

/**
 * @brief create subnet
 *
 * @param[in] subnet subnet information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_subnet_create (_In_ oci_subnet_t *subnet)
{
    api_ctxt_t    api_ctxt;
    sdk_ret_t     rv;

    api_ctxt.api_params = api::api_params_alloc(api::OBJ_ID_SUBNET,
                                                api::API_OP_CREATE);
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_CREATE;
        api_ctxt.obj_id = api::OBJ_ID_SUBNET;
        api_ctxt.api_params->subnet_info = *subnet;
        rv = api::g_api_engine.process_api(&api_ctxt);
        return rv;
    }
    return sdk::SDK_RET_OOM;
}

/**
 * @brief delete subnet
 *
 * @param[in] subnet_key subnet key
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_subnet_delete (_In_ oci_subnet_key_t *subnet_key)
{
    api_ctxt_t    api_ctxt;
    sdk_ret_t     rv;

    api_ctxt.api_params = api::api_params_alloc(api::OBJ_ID_SUBNET,
                                                api::API_OP_DELETE);
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_DELETE;
        api_ctxt.obj_id = api::OBJ_ID_SUBNET;
        api_ctxt.api_params->subnet_key = *subnet_key;
        rv = api::g_api_engine.process_api(&api_ctxt);
        return rv;
    }
    return sdk::SDK_RET_OOM;
}

/** @} */    // end of OCI_SUBNET_API
